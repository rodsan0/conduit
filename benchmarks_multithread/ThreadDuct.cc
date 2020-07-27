#include <thread>

#include "mpi.h"

#include "../source/Gatherer.h"
#include "../source/CircularIndex.h"
#include "../source/ThreadTeam.h"
#include "../source/Mesh.h"
#include "../source/latch.h"
#include "../source/TimeGuard.h"

#include "../source/mpi_utils.h"
#include "../source/pipe_utils.h"
#include "../source/numeric_cast.h"
#include "../source/mesh_utils.h"
#include "../source/thread_utils.h"
#include "../source/benchmark_utils.h"

#define MESSAGE_T int

void do_work(
  io_bundle_t<MESSAGE_T> bundle,
  latch & latch,
  Gatherer<MESSAGE_T> & gatherer
) {

  std::chrono::milliseconds duration; { const TimeGuard guard{duration};

  const bool is_producer = bundle.outputs.size();
  const bool is_consumer = bundle.inputs.size();
  const thread_id_t thread_id = get_thread_id();

  auto * const input = is_consumer ? &bundle.inputs[0].GetInput() : nullptr;
  auto * const output = is_producer ? &bundle.outputs[0].GetOutput() : nullptr;

  if (is_producer) output->MaybePut(thread_id);

  latch.arrive_and_wait();

  for (size_t rep = 0; rep < 1e7; ++rep) {
    if (is_producer) output->MaybePut(thread_id);
    if (is_consumer) do_not_optimize(
      input->GetCurrent()
    );
  }

  } // close TimeGuard

  gatherer.Put(duration.count());

}

void profile_thread_count(const size_t num_threads) {

  ThreadTeam team;

  Mesh mesh{
    make_ring_mesh<MESSAGE_T>(num_threads),
    assign_segregated<thread_id_t>()
  };

  Gatherer<MESSAGE_T> gatherer(MPI_INT);

  std::chrono::milliseconds duration; { const TimeGuard guard{duration};

  latch latch{numeric_cast<std::ptrdiff_t>(num_threads)};
  for (auto & node : mesh) {
    team.Add(
      [node, &latch, &gatherer](){ do_work(node, latch, gatherer); }
    );
  }

  team.Join();

  } // close TimeGuard

  auto res = gatherer.Gather();

  if (res) {

    std::cout << "threads: " << num_threads << std::endl;

    std::cout << "mean milliseconds:" << std::accumulate(
        std::begin(*res),
        std::end(*res),
        0.0
      ) / std::size(*res) << std::endl;;

    std::cout << "net milliseconds:" << duration.count() << std::endl;

  }


}

int main(int argc, char* argv[]) {

  int provided;
  verify(MPI_Init_thread(&argc, &argv, MPI_THREAD_SINGLE, &provided));
  emp_assert(provided >= MPI_THREAD_FUNNELED);

  for (size_t threads = 1; threads <= get_nproc(); threads*=2) {
    profile_thread_count(threads);
  }

  MPI_Finalize();

  return 0;
}
