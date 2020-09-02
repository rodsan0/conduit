#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <stddef.h>

#include <mpi.h>

#include "../../../../../../third-party/cereal/include/cereal/archives/binary.hpp"
#include "../../../../../../third-party/Empirical/source/base/assert.h"
#include "../../../../../../third-party/Empirical/source/base/vector.h"
#include "../../../../../../third-party/Empirical/source/tools/string_utils.h"

#include "../../../../distributed/mpi_utils.hpp"
#include "../../../../distributed/Request.hpp"
#include "../../../../utility/CircularIndex.hpp"
#include "../../../../utility/identity.hpp"
#include "../../../../utility/imemstream.hpp"
#include "../../../../utility/print_utils.hpp"
#include "../../../../utility/Uninitialized.hpp"

#include "../../../InterProcAddress.hpp"

#include "../../backend/MockBackEnd.hpp"

namespace uit {

/**
 * TODO
 *
 * @tparam ImplSpec class with static and typedef members specifying
 * implementation details for the conduit framework.
 */
template<typename ImplSpec>
class CerealRingIrecvDuct {

public:

  using BackEndImpl = uit::MockBackEnd<ImplSpec>;

private:

  using T = typename ImplSpec::T;
  constexpr inline static size_t N{ImplSpec::N};

  size_t pending_gets{};

  using buffer_t = emp::array<emp::vector<uit::Uninitialized<char>>, N>;
  buffer_t buffer{};

  using index_t = uit::CircularIndex<N>;
  index_t get_pos{};

  // cached unpacked value
  // initialize to value-constructed default
  mutable std::optional<T> cache{ std::in_place_t{} };

  const uit::InterProcAddress address;

  void PerformReceive(const MPI_Status& status) {
    const int msg_len = uit::get_count(status, MPI_BYTE);

    ++get_pos;
    buffer[get_pos].resize(msg_len);

    UIT_Recv(
      buffer[get_pos].data(), // void* buf: initial address of receive buffer
      msg_len, // int count: maximum number of elements in receive buffer
      MPI_BYTE,// MPI_Datatype datatype
      // datatype of each receive buffer element
      address.GetInletProc(), // int source: rank of source
      address.GetTag(), // int tag: message tag
      address.GetComm(), // MPI_Comm comm: communicator
      MPI_STATUS_IGNORE // MPI_Status *status: status object
    );

    // clear cached unpacked object
    cache.reset();

  }

  bool TryReceive() {

    MPI_Status status;
    int flag{};
    UIT_Iprobe(
      address.GetInletProc(), // int source
      // source rank, or MPI_ANY_SOURCE (integer)
      address.GetTag(), // int tag: tag value or MPI_ANY_TAG (integer)
      address.GetComm(), // MPI_Comm comm: communicator (handle)
      &flag, // int *flag
      // True if a message with the specified source, tag, and communicator
      // is available (logical)
      &status // MPI_Status *status: status object
    );

    if (flag) PerformReceive(status);

    return flag;

  }

  // returns number of requests fulfilled
  size_t FlushPendingReceives() {

    size_t num_received{};
    while( TryReceive() ) ++num_received;

    return num_received;

  }

  void UpdateCache() const {
    if (!cache.has_value()) {
      cache.emplace();

      uit::imemstream imemstream(
        reinterpret_cast<const char*>(buffer[get_pos].data()),
        buffer[get_pos].size()
      );
      cereal::BinaryInputArchive iarchive( imemstream );
      iarchive( cache.value() );
    }
  }

public:

  CerealRingIrecvDuct(
    const uit::InterProcAddress& address_,
    std::shared_ptr<BackEndImpl> back_end
  ) : address(address_) { }

  ~CerealRingIrecvDuct() {
    FlushPendingReceives();
  }

  [[noreturn]] bool TryPut(const T&) const {
    throw "Put called on CerealRingIrecvDuct";
  }

  /**
   * TODO.
   *
   * @param num_requested TODO.
   * @return number items consumed.
   */
  size_t TryConsumeGets(const size_t num_requested) {

    size_t requested_countdown{ num_requested };

    while ( requested_countdown && TryReceive() ) --requested_countdown;

    return num_requested - requested_countdown;
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  const T& Get() const {
    UpdateCache();
    return cache.value();
  }

  /**
   * TODO.
   *
   * @return TODO.
   */
  T& Get() {
    UpdateCache();
    return cache.value();
  }

  static std::string GetName() { return "CerealRingIrecvDuct"; }

  std::string ToString() const {
    std::stringstream ss;
    ss << GetName() << std::endl;
    ss << format_member("this", static_cast<const void *>(this)) << std::endl;
    ss << format_member("buffer_t buffer", buffer[0]) << std::endl;
    ss << format_member("size_t pending_gets", pending_gets) << std::endl;
    ss << format_member("InterProcAddress address", address) << std::endl;
    ss << format_member("size_t get_pos", get_pos);
    return ss.str();
  }

};

} // namespace uit