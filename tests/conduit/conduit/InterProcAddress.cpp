#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_DEFAULT_REPORTER "multiprocess"
#include "Catch/single_include/catch2/catch.hpp"

#include "uit/conduit/InterProcAddress.hpp"
#include "uit/distributed/MPIGuard.hpp"
#include "uit/distributed/MultiprocessReporter.hpp"

const uit::MPIGuard guard;

TEST_CASE("Test InterProcAddress") {

  // TODO flesh out stub test
  uit::InterProcAddress(0);

}