#include <thread>

#include <mpi.h>

#include "uitsl/chrono/TimeGuard.hpp"
#include "uit/conduit/Conduit.hpp"
#include "uit/config/ImplSpec.hpp"
#include "uitsl/debug/benchmark_utils.hpp"
#include "uitsl/debug/safe_cast.hpp"
#include "uit/mesh/Mesh.hpp"
#include "uitsl/mpi/mpi_utils.hpp"
#include "uitsl/nonce/CircularIndex.hpp"
#include "uitsl/parallel/ThreadTeam.hpp"
#include "uitsl/parallel/thread_utils.hpp"
#include "uitsl/polyfill/latch.hpp"

#define MESSAGE_T int

void do_work(std::latch & latch) {

  std::chrono::milliseconds duration; { const uitsl::TimeGuard guard{duration};

  latch.arrive_and_wait();

  uitsl::do_compute_work(1e7);

  } // close TimeGuard

}

void profile_thread_count(const size_t num_threads) {

  uitsl::ThreadTeam team;

  std::chrono::milliseconds duration; { const uitsl::TimeGuard guard{duration};

  std::latch latch{uitsl::safe_cast<std::ptrdiff_t>(num_threads)};
  for (size_t i = 0; i < num_threads; ++i) {
    team.Add( [&latch](){ do_work(latch); } );
  }

  team.Join();

  } // close TimeGuard

  std::cout << "t: " << duration.count() << std::endl;

}

int main(int argc, char* argv[]) {

  int provided;
  UITSL_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  emp_assert(provided >= MPI_THREAD_FUNNELED);

  for (size_t threads = 1; threads <= uitsl::get_nproc(); threads *= 2) {
    profile_thread_count(threads);
  }

  MPI_Finalize();

  return 0;
}
