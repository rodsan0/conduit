#include "uit/conduit/ImplSpec.hpp"
#include "uit/conduit/thread/put=growing+get=stepping/UnboundedMoodyCamelDuct.hpp"

using ImplSel = uit::ImplSelect<
  uit::SerialPendingDuct,
  uit::UnboundedMoodyCamelDuct
>;

#include "../ThreadDuct.hpp"