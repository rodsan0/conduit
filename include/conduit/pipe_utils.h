#pragma once

#include <iostream>
#include <utility>
#include <memory>
#include <atomic>

#include "Duct.h"
#include "Inlet.h"
#include "Outlet.h"

#include "config.h"

//TODO rename conduit
template<typename T, size_t N=DEFAULT_BUFFER>
std::pair<Inlet<T,N>, Outlet<T,N>> make_pipe() {

  std::tuple<std::shared_ptr<Duct<T, N>>> args{std::make_shared<Duct<T, N>>()};
  return std::pair<Inlet<T,N>, Outlet<T,N>>(
    std::piecewise_construct_t{},
    args,
    args
  );

}

template<typename T, size_t N=DEFAULT_BUFFER>
Inlet<T,N> make_sink() {
  return Inlet<T,N>(
    std::make_shared<Duct<T, N>>()
  );
}

template<typename T, size_t N=DEFAULT_BUFFER>
Outlet<T,N> make_source() {
  return Outlet<T,N>(
    std::make_shared<Duct<T, N>>()
  );
}