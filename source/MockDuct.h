#pragma once

#include "base/assert.h"
#include "tools/string_utils.h"

#include "CircularIndex.h"
#include "OccupancyCap.h"
#include "OccupancyGuard.h"

#include "print_utils.h"

template<typename T, size_t N>
class Duct;

template<typename T, size_t N>
class MockDuct {

  friend Duct<T, N>;

  using pending_t = size_t;
  using buffer_t = std::array<T, N>;

  pending_t pending{0};
  buffer_t buffer;

#ifndef NDEBUG
  mutable OccupancyCap cap{1};
#endif

public:

  //todo rename
  void Push() {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    emp_assert(
      pending < N,
      [](){ error_message_mutex.lock(); return "locked"; }(),
      emp::to_string("pending: ", pending)
    );

    ++pending;
  }

  //todo rename
  void Pop(const size_t count) {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    emp_assert(
      pending >= count,
      [](){ error_message_mutex.lock(); return "locked"; }(),
      emp::to_string("pending: ", pending),
      emp::to_string("count: ", count)
    );

    pending -= count;

  }

  size_t GetPending() const {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    return pending;
  }

  T GetElement(const size_t n) const {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    return buffer[n];
  }

  void SetElement(const size_t n, const T & val) {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    buffer[n] = val;
  }

  std::string ToString() const {

    #ifndef NDEBUG
      const OccupancyGuard guard{cap};
    #endif

    std::stringstream ss;
    ss << "MockDuct" << std::endl;
    ss << format_member("this", static_cast<const void *>(this)) << std::endl;
    ss << format_member("buffer_t buffer", buffer[0]) << std::endl;
    ss << format_member("pending_t pending", pending);
    return ss.str();
  }


};