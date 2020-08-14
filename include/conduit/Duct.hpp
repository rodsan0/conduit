#pragma once

#include <utility>
#include <variant>
#include <stddef.h>

#include "../../third-party/Empirical/source/base/assert.h"
#include "../../third-party/Empirical/source/tools/string_utils.h"
#include "../../third-party/Empirical/source/meta/TypePack.h"

//TODO don't include these all here
#include "thread/AtomicPendingDuct.hpp"
#include "intra/PendingDuct.hpp"
#include "thread/HeadTailDuct.hpp"
#include "thread/SPSCQueueDuct.hpp"
#include "proc/IsendDuct.hpp"
#include "proc/IrecvDuct.hpp"
#include "proc/RputDuct.hpp"
#include "proc/WindowDuct.hpp"

#include "../utility/print_utils.hpp"
#include "config.hpp"
#include "../distributed/mpi_utils.hpp"

namespace uit {

template<typename ImplSpec>
class Duct {

  using ducts_t = typename emp::TypePack<
    typename ImplSpec::IntraDuct,
    typename ImplSpec::ThreadDuct,
    typename ImplSpec::ProcInletDuct,
    typename ImplSpec::ProcOutletDuct
  >::make_unique;

  typename ducts_t::template apply<std::variant> impl;

  using T = typename ImplSpec::T;
  constexpr inline static size_t N{ImplSpec::N};

  std::string GetName() const {
    return emp::to_string(
      get_proc_id(),
      ":",
      GetUID()
    );
  }

public:

  using uid_t = std::uintptr_t;

  template <typename WhichDuct, typename... Args>
  void EmplaceDuct(Args&&... args) {
    impl.template emplace<WhichDuct>(std::forward<Args>(args)...);
  }

  //todo refactor to remove this
  //keep write/read position *within* duct
  void Initialize(const size_t write_position) {

    std::visit(
      [=](auto & arg) { arg.Initialize(write_position); },
      impl
    );

  }

  //todo rename
  void Push() {

    std::visit(
      [](auto & arg) { arg.Push(); },
      impl
    );

  }

  //todo rename
  void Pop(const size_t count) {

    std::visit(
      [count](auto & arg) { arg.Pop(count); },
      impl
    );

  }

  size_t GetPending() {
    return std::visit(
      [](auto & arg) -> size_t { return arg.GetPending(); },
      impl
    );
  }

  size_t GetAvailableCapacity() {
    return std::visit(
      [](auto & arg) -> size_t { return arg.GetAvailableCapacity(); },
      impl
    );
  }

  // todo rename
  T GetElement(const size_t n) const {
    return std::visit(
      [n](auto & arg) -> T { return arg.GetElement(n); },
      impl
    );
  }

  const void * GetPosition(const size_t n) const {
    return std::visit(
      [n](auto & arg) -> const void * { return arg.GetPosition(n); },
      impl
    );
  }

  void SetElement(const size_t n, const T & val) {
    std::visit(
      [n, &val](auto & arg) { arg.SetElement(n, val); },
      impl
    );
  }

  std::string GetType() const {
    return std::visit(
      [](auto & arg) -> std::string { return arg.GetType(); },
      impl
    );
  }

  uid_t GetUID() const { return reinterpret_cast<uid_t>(this); }

  std::string ToString() const {
    std::stringstream ss;
    ss << format_member("GetName()", GetName()) << std::endl;
    ss << format_member(
      "std::variant impl",
      std::visit(
        [](auto & arg) -> std::string { return arg.ToString(); },
        impl
      )
    );
    return ss.str();
  }

};

}
