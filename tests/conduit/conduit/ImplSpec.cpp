#define CATCH_CONFIG_MAIN
#include "Catch/single_include/catch2/catch.hpp"

#include "uit/conduit/ImplSpec.hpp"


TEST_CASE("Test ImplSelect") {

  uit::ImplSelect<>{};

}

TEST_CASE("Test ImplSpec") {

  uit::ImplSpec<char>{};

}
