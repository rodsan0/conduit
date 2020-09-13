#pragma once

#include <sstream>
#include <string>

#include <mpi.h>

#include "../../../third-party/Empirical/source/base/assert.h"

#include "../utility/print_utils.hpp"

#include "audited_routines.hpp"
#include "mpi_utils.hpp"

namespace uit {

int get_count(const MPI_Status& status, const MPI_Datatype& datatype) {
  int res;
  UIT_Get_count(
    &status, // const MPI_Status * status: return status of receive operation
    datatype, // MPI_Datatype datatype: datatype of each receive buffer element
    &res // int *count: number of received elements (integer)
  );
  emp_assert( res != MPI_UNDEFINED );
  return res;
}

bool test_cancelled(const MPI_Status& status) {
  int res;
  UIT_Test_cancelled(&status, &res);
  return res;
}

std::string to_string(const MPI_Status& status) {
  std::stringstream ss;
  ss << uit::format_member(
    "MPI_Get_count",
    uit::get_count(status, MPI_BYTE)
  ) << std::endl;
  ss << uit::format_member(
    "MPI_Test_cancelled",
    uit::test_cancelled(status)
  ) << std::endl;
  ss << uit::format_member(
    "int MPI_SOURCE",
    (int) status.MPI_SOURCE
  ) << std::endl;
  ss << uit::format_member(
    "int MPI_TAG",
    (int) status.MPI_TAG
  ) << std::endl;
  ss << uit::format_member(
    "int MPI_ERROR",
    (int) status.MPI_ERROR
  );
  return ss.str();
}

} // namespace uit