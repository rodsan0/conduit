#include <memory>

#define CATCH_CONFIG_DEFAULT_REPORTER "multiprocess"
#define CATCH_CONFIG_MAIN
#include "Catch/single_include/catch2/catch.hpp"

#include "Empirical/source/base/vector.h"

#include "uit/conduit/ImplSpec.hpp"
#include "uit/conduit/InterProcAddress.hpp"
#include "uit/conduit/proc/outlet/get=stepping+type=span/s::IprobeDuct.hpp"
#include "uit/mpi/MpiGuard.hpp"
#include "uit/debug/MultiprocessReporter.hpp"

const uit::MpiGuard guard;

TEST_CASE("Test s::IprobeDuct") {

  using ImplSpec = uit::ImplSpec<emp::vector<char>>;
  using BackEnd = uit::s::IprobeDuct<ImplSpec>::BackEndImpl;

  // TODO flesh out stub test
  uit::InterProcAddress address;
  std::shared_ptr<BackEnd> backing{ std::make_shared<BackEnd>() };
  uit::s::IprobeDuct<ImplSpec>{ address, backing };

}