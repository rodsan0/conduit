#include "uit/ducts/mock/ThrowDuct.hpp"
#include "uit/ducts/proc/put=growing+get=skipping+type=trivial/inlet=DequeIsend+outlet=BlockIrecv_t::IdiObiDuct.hpp"
#include "uit/setup/ImplSpec.hpp"

using ImplSel = uit::ImplSelect<
  uit::a::SerialPendingDuct,
  uit::ThrowDuct,
  uit::t::IdiObiDuct
>;

#define IMPL_NAME "inlet=DequeIsend+outlet=BlockIrecv_t::IdiObiDuct"

#include "../ProcDuct.hpp"
#include "../SkippingProcDuct.hpp"
