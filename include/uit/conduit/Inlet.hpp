#pragma once

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stddef.h>
#include <utility>

#include "../parallel/OccupancyCaps.hpp"
#include "../parallel/OccupancyGuard.hpp"
#include "../utility/CircularIndex.hpp"
#include "../utility/print_utils.hpp"

#include "config.hpp"
#include "Duct.hpp"

namespace uit {

/**
 * Input to conduit transmission.
 *
 * Allows user to initiate
 *
 *  - potentially-blocking, assured transmisison via `Put`, or
 *  - non-blocking, potentially-dropped transmission via `TryPut`.
 *
 * An `Inlet` holds a `std::shared_ptr` to a `Duct` object, which manages data
 * transmission from the `Inlet`.
 *
 * An `Inlet`'s underlying `Duct` may be altered or replaced at runtime, for
 * example to provide thread-safe or process-safe transmission.
 *
 * - `EmplaceDuct` emplaces a new transmission implementation within
 *   the  existing `Duct` object. (See `include/conduit/Duct.hpp` for details.)
 * - `SplitDuct` makes a new `Duct` and points the `Inlet`'s `std::shared_ptr`
 *   to that `Duct`.
 *
 * If an `Outlet` holds a `std::shared_ptr` to the `Inlet`'s `Duct`, under an
 * `EmplaceDuct` call the `Duct`'s change in transmission implementation will
 * be visible to the `Outlet` and `Inlet` and the `Outlet` will still share a
 * `Duct`. However, under a `SplitDuct` call that `Outlet`'s `Duct` will be
 * unaffected. After a `SplitDuct` call, the `Inlet` and `Outlet` will hold
 * `std::shared_ptr`'s to separate `Duct`s.
 *
 * @tparam ImplSpec class with static and typedef members specifying
 *   implementation details for the conduit framework. See
 *   `include/conduit/ImplSpec.hpp`.
 *
 * @note End users should probably never have to directly instantiate this
 *   class. The `Conduit`, `Sink`, and `Source` classes take care of creating a
 *   `Duct` and tying it to an `Inlet` and/or `Outlet`. Better yet, the
 *   `MeshTopology` interface allows end users to construct a conduit network
 *    in terms of a connection topology and a mapping to assign nodes to
 *    threads and processes without having to manually construct `Conduits` and
 *    emplace necessary thread-safe and/or process-safe `Duct` implementations.
 */
template<typename ImplSpec>
class Inlet {

  using T = typename ImplSpec::T;
  constexpr inline static size_t N{ImplSpec::N};

#ifndef NDEBUG
  uit::OccupancyCaps caps;
#endif

  using index_t = uit::CircularIndex<N>;

  /// TODO.
  using duct_t = internal::Duct<ImplSpec>;
  std::shared_ptr<duct_t> duct;

  /// How many put operations have been performed?
  size_t successful_put_count{};

  /// How many times has SurePut blocked?
  size_t blocked_put_count{};

  // How many TryPut calls have dropped?
  size_t dropped_put_count{};

  /**
   * TODO.
   *
   * @param val
   */
  void Put(const T& val) { duct->Put(val); }

public:

  /**
   * TODO.
   *
   * @param duct_ TODO.
   */
  Inlet(
    std::shared_ptr<duct_t> duct_
  ) : duct(duct_) { ; }

  // potentially blocking
  /**
   * TODO.
   *
   * @param val TODO.
   */
  void SurePut(const T& val) {
    #ifndef NDEBUG
      const uit::OccupancyGuard guard{caps.Get("SurePut", 1)};
    #endif

    blocked_put_count += !IsReadyForPut();
    while (!IsReadyForPut());

    Put(val);

  }

  // non-blocking
  /**
   * TODO.
   *
   * @param val TODO.
   */
  bool TryPut(const T& val) {
    #ifndef NDEBUG
      const uit::OccupancyGuard guard{caps.Get("TryPut", 1)};
    #endif

    if (!IsReadyForPut()) {
      ++dropped_put_count;
      return false;
    }

    Put(val);

    return true;

  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  bool IsReadyForPut() { return duct->IsReadyForPut(); }

  /**
   * TODO.
   *
   * @return TODO.
   */
  size_t GetSuccessfulPutCount() const { return successful_put_count; }

  /**
   * TODO.
   *
   * @return TODO.
   */
  size_t GetBlockedPutCount() const { return blocked_put_count; }

  /**
   * TODO.
   *
   * @return TODO.
   */
  size_t GetDroppedPutCount() const { return dropped_put_count; }

  /**
   * TODO.
   *
   * @tparam WhichDuct
   * @tparam Args
   * @param args
   */
  template <typename WhichDuct, typename... Args>
  void EmplaceDuct(Args&&... args) {
    duct->template EmplaceImpl<WhichDuct>(std::forward<Args>(args)...);
  }

  /**
   * TODO.
   *
   * @tparam WhichDuct
   * @tparam Args
   * @param args
   */
  template <typename WhichDuct, typename... Args>
  void SplitDuct(Args&&... args) {
    duct = std::make_shared<duct_t>(
      std::in_place_type_t<WhichDuct>{},
      std::forward<Args>(args)...
    );
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  typename duct_t::uid_t GetDuctUID() const {
    return duct->GetUID();
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  std::string ToString() const {
    std::stringstream ss;
    ss << format_member("duct_t duct", *duct) << std::endl;
    ss << format_member(
      "size_t successful_put_count",
      successful_put_count
    ) << std::endl;
    ss << format_member(
      "size_t dropped_put_count",
      dropped_put_count
    ) << std::endl;;
    ss << format_member(
      "size_t blocked_put_count",
      blocked_put_count
    );
    return ss.str();
  }


};

} // namespace uit