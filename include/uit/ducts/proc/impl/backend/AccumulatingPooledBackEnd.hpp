#pragma once
#ifndef UIT_DUCTS_PROC_IMPL_BACKEND_ACCUMULATINGPOOLEDBACKEND_HPP_INCLUDE
#define UIT_DUCTS_PROC_IMPL_BACKEND_ACCUMULATINGPOOLEDBACKEND_HPP_INCLUDE

#include "../../../../../../third-party/Empirical/include/emp/base/vector.hpp"

#include "../../../../../uitsl/datastructs/VectorMap.hpp"

#include "../../../../setup/InterProcAddress.hpp"

#include "impl/InletMemoryAccumulatingPool.hpp"
#include "impl/OutletMemoryPool.hpp"
#include "impl/PoolSpec.hpp"

#include "RuntimeSizeBackEnd.hpp"

namespace uit {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<
  typename ImplSpec,
  template<typename> typename ProcDuct
>
// TODO combine implementation with PooledBackEnd to reduce code duplication
class AccumulatingPooledBackEnd {

  using T = typename ImplSpec::T;

  using address_t = uit::InterProcAddress;

  using PoolSpec_t = uit::PoolSpec<ImplSpec, ProcDuct>;

public:

  using inlet_pool_t = uit::InletMemoryAccumulatingPool<PoolSpec_t>;
  using outlet_pool_t = uit::OutletMemoryPool<PoolSpec_t>;

private:

  // thread_id of caller -> proc_id of target -> inlet pool
  uitsl::VectorMap<
    uitsl::thread_id_t,
    uitsl::VectorMap<
      uitsl::proc_id_t,
      inlet_pool_t
    >
  > inlet_pools;

  // thread_id of caller -> proc_id of target -> outlet pool
  uitsl::VectorMap<
    uitsl::thread_id_t,
    uitsl::VectorMap<
      uitsl::proc_id_t,
      outlet_pool_t
    >
  > outlet_pools;

  bool AreAllInletPoolsInitialized() const {

    // check that all windows are in the same initialization state
    emp_assert( std::all_of(
      std::begin(inlet_pools), std::end(inlet_pools),
      [this](const auto& map_pair){
        const auto& [key, map] = map_pair;
        return std::all_of(
          std::begin(map), std::end(map),
          [this](const auto& pool_pair) {
            const auto& [key, pool] = pool_pair;
            const auto& rep = inlet_pools.begin()->second.begin()->second;
            return pool.IsInitialized() == rep.IsInitialized();
          }
        );
      }
    ) );


    return std::any_of(
      std::begin(inlet_pools), std::end(inlet_pools),
      [this](const auto& map_pair){
        const auto& [key, map] = map_pair;
        return std::any_of(
          std::begin(map), std::end(map),
          [this](const auto& pool_pair) {
            const auto& [key, pool] = pool_pair;
            return pool.IsInitialized();
          }
        );
      }
    );
  }

  bool AreAllOutletPoolsInitialized() const {

    // check that all windows are in the same initialization state
    emp_assert( std::all_of(
      std::begin(outlet_pools), std::end(outlet_pools),
      [this](const auto& map_pair){
        const auto& [key, map] = map_pair;
        return std::all_of(
          std::begin(map), std::end(map),
          [this](const auto& pool_pair) {
            const auto& [key, pool] = pool_pair;
            const auto& rep = outlet_pools.begin()->second.begin()->second;
            return pool.IsInitialized() == rep.IsInitialized()  ;
          }
        );
      }
    ) );

    return std::any_of(
      std::begin(outlet_pools), std::end(outlet_pools),
      [this](const auto& map_pair){
        const auto& [key, map] = map_pair;
        return std::any_of(
          std::begin(map), std::end(map),
          [this](const auto& pool_pair) {
            const auto& [key, pool] = pool_pair;
            return pool.IsInitialized();
          }
        );
      }
    );
  }

  bool IsInitialized() {
    emp_assert(
      AreAllInletPoolsInitialized() == AreAllOutletPoolsInitialized()
      || inlet_pools.empty()
      || outlet_pools.empty()
    );
    return AreAllInletPoolsInitialized() || AreAllOutletPoolsInitialized();
  }

  bool IsEmpty() {
    return outlet_pools.empty() && inlet_pools.empty();
  }

public:

  void RegisterInletSlot(const address_t& address) {
    emp_assert( !IsInitialized() );
    inlet_pools[
      address.GetInletThread()
    ][
      address.GetOutletProc()
    ].Register(address);
  }

  void RegisterOutletSlot(const address_t& address) {
    emp_assert( !IsInitialized() );
    outlet_pools[
      address.GetOutletThread()
    ][
      address.GetInletProc()
    ].Register(address);
  }

  void Initialize() {
    emp_assert( !IsInitialized() );

    std::set<uitsl::thread_id_t> thread_ids;
    for (auto& [thread_id, __] : inlet_pools) thread_ids.insert(thread_id);
    for (auto& [thread_id, __] : outlet_pools) thread_ids.insert(thread_id);

    for (auto thread_id : thread_ids) InitializeThread( thread_id );

    emp_assert( IsInitialized() || IsEmpty() );
  }

  void InitializeThread(const uitsl::thread_id_t thread_id) {

    auto backend{ std::make_shared<typename PoolSpec_t::ProcBackEnd>() };

    if ( inlet_pools.count(thread_id) ) {
      for (auto& [__, pool] : inlet_pools[thread_id]) {
        pool.Initialize( backend );
      }
    }

    if ( outlet_pools.count(thread_id) ) {
      for (auto& [__, pool] : outlet_pools[thread_id]) {
        pool.Initialize( backend );
      }
    }

    backend->Initialize();

  }

  inlet_pool_t& GetInletPool(const address_t& address) {
    emp_assert( IsInitialized() );

    auto& pool = inlet_pools.at(
      address.GetInletThread()
    ).at(
      address.GetOutletProc()
    );

    emp_assert( pool.IsInitialized(), pool.GetSize() );

    return pool;
  }

  outlet_pool_t& GetOutletPool(const address_t& address) {
    emp_assert( IsInitialized() );

    auto& pool = outlet_pools.at(
      address.GetOutletThread()
    ).at(
      address.GetInletProc()
    );

    emp_assert( pool.IsInitialized() );

    return pool;
  }

  constexpr static bool CanStep() { return outlet_pool_t::CanStep(); }

};

} // namespace uit

#endif // #ifndef UIT_DUCTS_PROC_IMPL_BACKEND_ACCUMULATINGPOOLEDBACKEND_HPP_INCLUDE
