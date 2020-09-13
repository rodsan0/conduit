#include "uit/conduit/ImplSpec.hpp"
#include "uit/conduit/intra/put=growing+get=skipping+type=any/a::SconceDuct.hpp"
#include "uit/conduit/mock/ThrowDuct.hpp"

using ImplSel = uit::ImplSelect<
  uit::a::SconceDuct,
  uit::ThrowDuct,
  uit::ThrowDuct
>;

#include "../IntraDuct.hpp"
#include "../ValueIntraDuct.hpp"