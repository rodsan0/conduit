#pragma once

#include <utility>
#include <memory>
#include <stddef.h>

#include "Duct.hpp"
#include "Outlet.hpp"

namespace uit {

/**
 * Creates an ucoupled `Outlet`.
 *
 * Potentially useful for inter-process transmission, where a conceptually-
 * coupled `Inlet` and `Outlet` must be constructed within separate memory
 * spaces.
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework. See
 *   `include/conduit/ImplSpec.hpp`.
 */
template<typename ImplSpec>
class Source {

  /**
   * TODO.
   *
   * @return TODO.
   */
  uit::Outlet<ImplSpec> outlet;

public:

  /**
   * Copy constructor.
   */
  Source(Source& other) = default;

  /**
   * Copy constructor.
   */
  Source(const Source& other) = default;

  /**
   * Move constructor.
   */
  Source(Source&& other) = default;

  /**
   * TODO
   */
  template <typename... Args>
  Source(Args&&... args) : outlet(
    std::make_shared<internal::Duct<ImplSpec>>(
      std::forward<Args>(args)...
    )
  ) { ; }

  // for structured bindings
  /**
   * TODO.
   *
   * @return TODO.
   */
  template <size_t N>
  decltype(auto) get() const {
    if constexpr (N == 0) return outlet;
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  uit::Outlet<ImplSpec>& GetOutlet() {
    return outlet;
  }

  // TODO implicit conversion operator?

};

} // namespace uit

// for structured bindings
namespace std {

  template<typename ImplSpec>
  struct tuple_size<uit::Source<ImplSpec>>
    : std::integral_constant<size_t, 1>{};

  template<typename ImplSpec, size_t N>
  struct tuple_element<N, uit::Source<ImplSpec>> {

    using type = decltype(
      std::declval<uit::Source<ImplSpec>>().template get<N>()
    );
  };

} // namespace std