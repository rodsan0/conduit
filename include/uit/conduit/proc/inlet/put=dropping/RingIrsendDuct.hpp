#pragma once

#include "../../../../distributed/IrsendFunctor.hpp"

#include "impl/RingImmediateSendDuct.hpp"

namespace uit {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<typename ImplSpec>
class RingIrsendDuct
: public uit::internal::RingImmediateSendDuct<
  uit::IrsendFunctor,
  ImplSpec
> {

  // inherit parent's constructors
  // adapted from https://stackoverflow.com/a/434784
  using parent_t = uit::internal::RingImmediateSendDuct<
    uit::IrsendFunctor,
    ImplSpec
  >;
  using parent_t::parent_t;


  /**
   * TODO.
   *
   * @return TODO.
   */
  static std::string GetName() { return "RingIrsendDuct"; }

};

} // namespace uit
