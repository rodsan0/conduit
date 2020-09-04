#include <limits>
#include <utility>

#define CATCH_CONFIG_MAIN
#include "Catch/single_include/catch2/catch.hpp"

#include "Empirical/source/base/array.h"

#include "uit/utility/NamedArrayElement.hpp"

// adapted from http://cplusplus.bordoon.com/namedArrayElements.html
struct NamedArray {

  union {

    double m_array[2];

    uit::NamedArrayElement<0, double> foo;
    uit::NamedArrayElement<1, double> bar;

  };

  double &operator[](const int i) { return m_array[i]; }

  const double &operator[](const int i) const { return m_array[i]; }

};

TEST_CASE("NamedArrayElement") {

  NamedArray arr;

  arr.foo = 42;
  arr.bar = 24;

  REQUIRE( arr[0] == 42 );
  REQUIRE( arr[1] == 24 );

  std::swap(arr[0], arr[1]);

  REQUIRE( arr[0] == 24 );
  REQUIRE( arr[1] == 42 );

}