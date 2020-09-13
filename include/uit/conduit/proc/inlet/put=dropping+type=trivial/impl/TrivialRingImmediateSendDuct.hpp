#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <stddef.h>

#include <mpi.h>

#include "../../../../../../../third-party/Empirical/source/base/assert.h"
#include "../../../../../../../third-party/Empirical/source/tools/string_utils.h"

#include "../../../../../debug/err_audit.hpp"
#include "../../../../../mpi/mpi_utils.hpp"
#include "../../../../../mpi/Request.hpp"
#include "../../../../../utility/CircularIndex.hpp"
#include "../../../../../utility/print_utils.hpp"
#include "../../../../../utility/RingBuffer.hpp"

#include "../../../../InterProcAddress.hpp"

#include "../../../backend/MockBackEnd.hpp"

namespace uit {
namespace internal {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<typename ImmediateSendFunctor, typename ImplSpec>
class TrivialRingImmediateSendDuct {

public:

  using BackEndImpl = uit::MockBackEnd<ImplSpec>;

private:

  using T = typename ImplSpec::T;
  constexpr inline static size_t N{ImplSpec::N};

  using buffer_t = uit::RingBuffer<std::tuple<T, uit::Request>, N>;
  buffer_t buffer{};

  const uit::InterProcAddress address;

  void PostSendRequest() {

    emp_assert( uit::test_null( std::get<uit::Request>( buffer.GetHead() ) ) );
    ImmediateSendFunctor{}(
      &std::get<T>( buffer.GetHead() ),
      sizeof(T),
      MPI_BYTE,
      address.GetOutletProc(),
      address.GetTag(),
      address.GetComm(),
      &std::get<uit::Request>( buffer.GetHead() )
    );
    emp_assert(!uit::test_null(std::get<uit::Request>( buffer.GetHead() )));

  }

  bool TryFinalizeSend() {
    emp_assert( !uit::test_null( std::get<uit::Request>( buffer.GetTail() ) ) );

    if (uit::test_completion( std::get<uit::Request>( buffer.GetTail() ) )) {
      emp_assert( uit::test_null( std::get<uit::Request>(buffer.GetTail()) ) );
      uit::err_audit(!   buffer.PopTail()   );
      return true;
    } else return false;
  }

  void CancelPendingSend() {
    emp_assert( !uit::test_null( std::get<uit::Request>( buffer.GetTail() ) ) );

    UIT_Cancel( &std::get<uit::Request>( buffer.GetTail() ) );
    UIT_Request_free( &std::get<uit::Request>( buffer.GetTail() ) );

    emp_assert( uit::test_null( std::get<uit::Request>( buffer.GetTail() ) ) );

    uit::err_audit(!   buffer.PopTail()   );
  }

  void FlushFinalizedSends() { while (buffer.GetSize() && TryFinalizeSend()); }

  /**
   * TODO.
   *
   * @param val TODO.
   */
  void DoPut(const T& val) {
    emp_assert( buffer.GetSize() < N );

    uit::err_audit(!   buffer.PushHead()   );

    std::get<T>( buffer.GetHead() ) = val;

    PostSendRequest();
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  bool IsReadyForPut() {
    FlushFinalizedSends();
    return buffer.GetSize() < N;
  }

public:

  TrivialRingImmediateSendDuct(
    const uit::InterProcAddress& address_,
    std::shared_ptr<BackEndImpl> back_end
  ) : address(address_)
  { ; }

  ~TrivialRingImmediateSendDuct() {
    FlushFinalizedSends();
    while ( buffer.GetSize() ) CancelPendingSend();
  }

  /**
   * TODO.
   *
   * @param val TODO.
   */
  bool TryPut(const T& val) {
    if (IsReadyForPut()) { DoPut(val); return true; }
    else return false;
  }

  /**
   * TODO.
   */
  bool Flush() const { return true; }

  [[noreturn]] size_t TryConsumeGets(size_t) const {
    throw "ConsumeGets called on TrivialRingImmediateSendDuct";
  }

  [[noreturn]] const T& Get() const {
    throw "Get called on TrivialRingImmediateSendDuct";
  }

  [[noreturn]] T& Get() {
    throw "Get called on TrivialRingImmediateSendDuct";
  }

  static std::string GetType() { return "TrivialRingImmediateSendDuct"; }

  std::string ToString() const {
    std::stringstream ss;
    ss << GetType() << std::endl;
    ss << format_member("this", static_cast<const void *>(this)) << std::endl;
    ss << format_member("InterProcAddress address", address) << std::endl;
    return ss.str();
  }

};

} // namespace internal
} // namespace uit