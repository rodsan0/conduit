#include "conduit/ImplSpec.hpp"
#include "conduit/mock/ThrowDuct.hpp"
#include "conduit/intra/HeadTailDuct.hpp"

using ImplSel = uit::ImplSelect<
  uit::HeadTailDuct,
  uit::ThrowDuct,
  uit::ThrowDuct
>;

#include "IntraDuct.hpp"
