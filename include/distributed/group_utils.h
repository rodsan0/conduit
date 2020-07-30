#pragma once

#include <set>

#include "mpi.h"

#include "base/assert.h"
#include "tools/math.h"

#include "utility/math_utils.h"
#include "utility/safe_compare.h"
#include "utility/print_utils.h"

#include "mpi_utils.h"

MPI_Group make_group(
  emp::vector<proc_id_t> ranks,
  const MPI_Group source=comm_to_group(MPI_COMM_WORLD)
) {

  std::sort(std::begin(ranks), std::end(ranks));
  const auto last{ std::unique(std::begin(ranks), std::end(ranks)) };
  ranks.erase(last, std::end(ranks));

  emp_assert(std::set<proc_id_t>(
    std::begin(ranks),
    std::end(ranks)
  ).size() == ranks.size(), to_string(ranks));
  emp_assert(std::all_of(
    std::begin(ranks),
    std::end(ranks),
    [&](const auto & rank){
      return safe_less(rank, group_size(source)) && rank >= 0;
    }
  ), to_string(ranks));

  MPI_Group res;
  verify(MPI_Group_incl(
    source, // MPI_Group group
    ranks.size(), // int n
    ranks.data(), // const int ranks[]
    &res // MPI_Group * newgroup
  ));
  return res;
}
