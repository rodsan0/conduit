#pragma once

#include "SendDuct.hpp"
#include "IrecvDuct.hpp"

namespace uit {

template<typename ImplSpec>
struct MsgDuct {

  using InletImpl = uit::SendDuct<ImplSpec>;
  using OutletImpl = uit::IrecvDuct<ImplSpec>;

};

}
