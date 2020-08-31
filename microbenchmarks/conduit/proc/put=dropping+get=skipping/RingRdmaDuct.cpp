#include "uit/conduit/ImplSpec.hpp"
#include "uit/conduit/proc/put=dropping+get=skipping/RingRdmaDuct.hpp"

using ImplSel = uit::ImplSelect<
  uit::SerialPendingDuct,
  uit::AtomicPendingDuct,
  uit::RingRdmaDuct
>;

#include "../ProcDuct.hpp"
