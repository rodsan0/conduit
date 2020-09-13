#pragma once

#include <algorithm>
#include <mutex>
#include <optional>
#include <set>

#include "../../../../../../third-party/Empirical/source/base/assert.h"

#include "../../../InterProcAddress.hpp"
#include "../../../Inlet.hpp"
#include "../../../Sink.hpp"

namespace uit {

template<typename PoolSpec>
class InletMemoryPool {

  using address_t = uit::InterProcAddress;
  std::set<address_t> addresses;

  std::optional<uit::Inlet<PoolSpec>> inlet;

  using T = typename PoolSpec::T;
  T buffer{};

  // incremented every time TryPut is called
  // then reset to zero once every member of the pool has called
  size_t flush_counter{};
  // did the most recent put request succeed?
  // (can we put new entries in the buffer?)
  bool put_status{ true };

  #ifndef NDEBUG
    std::unordered_set<size_t> index_checker;
  #endif

  using value_type = typename PoolSpec::T::value_type;

  void Flush() {
    if ( ++flush_counter == addresses.size() ) {
      flush_counter = 0;
      put_status = inlet->TryPut( std::move(buffer) );
      buffer.resize( addresses.size() );
      #ifndef NDEBUG
        index_checker.clear();
      #endif
    }
  }

  void CheckCallingProc() const {
    const auto& rep = *addresses.begin();
    emp_assert( rep.GetInletProc() == uit::get_rank( rep.GetComm() ) );
  }

public:

  bool IsInitialized() const { return inlet.has_value(); }

  size_t GetSize() const { return addresses.size(); }

  /// Retister a duct for an entry in the pool.
  void Register(const address_t& address) {
    emp_assert( !IsInitialized() );
    emp_assert( !addresses.count(address) );
    addresses.insert(address);
  }

  /// Get index of this duct's entry in the pool.
  size_t Lookup(const address_t& address) const {
    emp_assert( IsInitialized() );
    emp_assert( addresses.count(address) );
    CheckCallingProc();
    return std::distance( std::begin(addresses), addresses.find(address) );
  }

  /// Get the querying duct's current value from the underlying duct.
  bool TryPut(const value_type& val, const size_t index) {
    emp_assert( IsInitialized() );
    CheckCallingProc();

    if (put_status) {
      buffer[index] = val;
      emp_assert( index_checker.insert(index).second );
    }

    const bool res = put_status;

    Flush();

    return res;

  }

  /// Call after all members have requested a position in the pool.
  void Initialize() {

    emp_assert( !IsInitialized() );

    emp_assert( std::all_of(
      std::begin(addresses),
      std::end(addresses),
      [this](const auto& addr){ return (
          addr.GetOutletProc() == addresses.begin()->GetOutletProc()
          && addr.GetInletThread() == addresses.begin()->GetInletThread()
          && addr.GetComm() == addresses.begin()->GetComm()
        );
      }
    ) );

    buffer.resize( addresses.size() );
    auto backend = std::make_shared<
      typename PoolSpec::ProcBackEnd
    >( addresses.size() );

    auto sink = uit::Sink<PoolSpec>{
      std::in_place_type_t<
        typename PoolSpec::ProcInletDuct
      >{},
      *addresses.begin(),
      backend
    };

    backend->Initialize();

    inlet = sink.GetInlet();

  }

};

} // namespace uit