#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <stddef.h>

#include <mpi.h>

#include "../../../../../../third-party/Empirical/source/base/assert.h"
#include "../../../../../../third-party/Empirical/source/tools/string_utils.h"

#include "../../../../debug/err_audit.hpp"
#include "../../../../mpi/mpi_utils.hpp"
#include "../../../../utility/SiftingArray.hpp"
#include "../../../../utility/print_utils.hpp"
#include "../../../../utility/RingBuffer.hpp"

#include "../../../InterProcAddress.hpp"

#include "../../backend/MockBackEnd.hpp"

namespace uit {
namespace t {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<typename ImplSpec>
class RingIrecvDuct {

public:

  using BackEndImpl = uit::MockBackEnd<ImplSpec>;

private:

  using T = typename ImplSpec::T;
  constexpr inline static size_t N{ImplSpec::N};

  // one extra in the data buffer to hold the current get
  uit::RingBuffer<T, N + 1> data;
  uit::SiftingArray<MPI_Request, N> requests;

  const uit::InterProcAddress address;

  void PostReceiveRequest() {
    uit::err_audit(!
      data.PushHead()
    );
    requests.PushBack( MPI_REQUEST_NULL );

    emp_assert( uit::test_null( requests.Back() ) );
    UIT_Irecv(
      &data.GetHead(),
      sizeof(T),
      MPI_BYTE,
      address.GetInletProc(),
      address.GetTag(),
      address.GetComm(),
      &requests.Back()
    );
    emp_assert( !uit::test_null( requests.Back() ) );

  }

  void CancelReceiveRequest() {
    emp_assert( !uit::test_null( requests.Back() ) );

    UIT_Cancel(  &requests.Back() );
    UIT_Request_free( &requests.Back() );

    emp_assert( uit::test_null( requests.Back() ) );

    uit::err_audit(!  data.PopTail()  );
    uit::err_audit(!  requests.PopBack()  );

  }

  void CancelOpenReceiveRequests() {
    while ( requests.GetSize() ) CancelReceiveRequest();
  }

  void TestRequests() {

    // MPICH Testsome returns negative outcount for zero count calls
    // so let's boogie out early to avoid drama
    if (requests.GetSize() == 0) return;

    emp_assert( std::none_of(
      std::begin(requests),
      std::end(requests),
      [](const auto& req){ return uit::test_null( req ); }
    ) );

    thread_local emp::array<int, N> out_indices; // ignored
    int num_received; // ignored

    UIT_Testsome(
      requests.GetSize(), // int count
      requests.GetData(), // MPI_Request array_of_requests[]
      &num_received, // int *outcount
      out_indices.data(), // int *indices
      MPI_STATUSES_IGNORE // MPI_Status array_of_statuses[]
    );

    emp_assert( num_received >= 0 );
    emp_assert( static_cast<size_t>(num_received) <= requests.GetSize() );

  }

  void TryFulfillReceiveRequests() {
    TestRequests();
    requests.SiftByValue(
      [](const auto& request){ return request != MPI_REQUEST_NULL; }
    );
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  size_t CountUnconsumedGets() {
    TryFulfillReceiveRequests();
    return data.GetSize() - requests.GetSize() - 1;
  }

public:

  RingIrecvDuct(
    const uit::InterProcAddress& address_,
    std::shared_ptr<BackEndImpl> back_end
  ) : address(address_) {

    data.PushHead( T{} ); // value-initialized initial Get item
    for (size_t i = 0; i < N; ++i) PostReceiveRequest();
    emp_assert( std::none_of(
      std::begin(requests),
      std::end(requests),
      [](const auto& req){ return uit::test_null( req ); }
    ) );
  }

  ~RingIrecvDuct() {
    while ( CountUnconsumedGets() ) TryConsumeGets( CountUnconsumedGets() );
    CancelOpenReceiveRequests();
  }

  [[noreturn]] bool TryPut(const T&) const {
    throw "TryPut called on RingIrecvDuct";
  }

  /**
   * TODO.
   *
   */
  [[noreturn]] bool Flush() const { throw "Flush called on RingIrecvDuct"; }

  /**
   * TODO.
   *
   * @param num_requested TODO.
   * @return number items consumed.
   */
  size_t TryConsumeGets(const size_t num_requested) {

    size_t requested_countdown{ num_requested };
    size_t batch_countdown{ CountUnconsumedGets() };
    bool full_batch = (batch_countdown == N);

    while ( batch_countdown && requested_countdown ) {

      --batch_countdown;
      --requested_countdown;
      uit::err_audit(!   data.PopTail()   );
      PostReceiveRequest();

      if (full_batch && batch_countdown == 0) {
        batch_countdown = CountUnconsumedGets();
        full_batch = (batch_countdown == N);
      }
    }

    const size_t num_consumed = num_requested - requested_countdown;
    return num_consumed;
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  const T& Get() const { return data.GetTail(); }

  /**
   * TODO.
   *
   * @return TODO.
   */
  T& Get() { return data.GetTail(); }

  static std::string GetName() { return "RingIrecvDuct"; }

  std::string ToString() const {
    std::stringstream ss;
    ss << GetName() << std::endl;
    ss << format_member("this", static_cast<const void *>(this)) << std::endl;
    ss << format_member("InterProcAddress address", address) << std::endl;
    return ss.str();
  }

};

} // namespace t
} // namespace uit