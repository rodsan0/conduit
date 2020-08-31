#pragma once

#include <stddef.h>

#include "../proc/backend/MockBackEnd.hpp"

namespace uit {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<typename ImplSpec>
class NopDuct {

  using T = typename ImplSpec::T;

public:

  using InletImpl = NopDuct<ImplSpec>;
  using OutletImpl = NopDuct<ImplSpec>;
  using BackEndImpl = uit::MockBackEnd<ImplSpec>;

  /**
   * TODO.
   */
  template <typename... Args>
  NopDuct(Args&&... args) { ; }

  /**
   * TODO.
   *
   * @param val TODO.
   */
  void Put(const T&) { ; }

  /**
   * TODO.
   *
   * @return TODO.
   */
  bool IsReadyForPut() { return true; }

  /**
  * TODO.
  *
  * @return TODO.
   */
  const T& Get() { const static T val{}; return val; }

  /**
   * TODO.
   *
   * @return TODO.
   */
  size_t TryConsumeGets(size_t) { return 0; }

  /**
   * TODO.
   */
  static std::string GetName() { return "NopDuct"; }

  /**
   * TODO.
   */
  std::string ToString() const { return std::string{}; }

};

} // namespace uit