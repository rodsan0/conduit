#pragma once

#include <mpi.h>

namespace uit {

class Request {

  MPI_Request req{ MPI_REQUEST_NULL };

public:

  Request() { ; }
  Request(const MPI_Request& req_) : req(req_) { ; }

  operator const MPI_Request&() const { return req; }
  operator MPI_Request&() { return req; }

  MPI_Request* operator&() { return &req; }

};

} // namespace uit
