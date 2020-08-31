#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DEFAULT_REPORTER "multiprocess"
#include "Catch/single_include/catch2/catch.hpp"

#include "uit/conduit/ImplSpec.hpp"
#include "uit/conduit/Sink.hpp"
#include "uit/distributed/MPIGuard.hpp"
#include "uit/distributed/MultiprocessReporter.hpp"

const uit::MPIGuard guard;

TEST_CASE("Test Sink") {

  // TODO flesh out stub test
  uit::Sink<uit::ImplSpec<char>> sink;
  sink.get<0>();

  [[maybe_unused]] auto& [inlet] = sink;

}
