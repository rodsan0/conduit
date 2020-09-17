#pragma once

#include <mpi.h>

#include "audited_routines.hpp"
#include "mpi_utils.hpp"

namespace uitsl {

bool test_completion(const MPI_Request& request) {
  int flag{};
  // on MPI spec, request is listed as an input param but not taken as const ptr
  UITSL_Test(
    const_cast<MPI_Request*>(&request), // MPI_Request * request
    &flag, // int *flag
    MPI_STATUS_IGNORE // MPI_Status * status
  );
  return flag;
}

bool test_null(const MPI_Request& request) {
  return request == MPI_REQUEST_NULL;
}

} // namespace uitsl
