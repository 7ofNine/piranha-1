/* Copyright 2009-2017 Francesco Biscani (bluescarni@gmail.com)

This file is part of the Piranha library.

The Piranha library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The Piranha library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the Piranha library.  If not,
see https://www.gnu.org/licenses/. */

#include <piranha/invert.hpp>


#include <cmath>
#include <string>
#include <type_traits>

#include <mp++/config.hpp>

#include <piranha/integer.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/monomial.hpp>
#include <piranha/poisson_series.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif

#include "catch.hpp"

using namespace piranha;

using math::invert;

#if defined(MPPP_WITH_MPFR)

static inline real operator"" _r(const char *s)
{
    return real(s, 100);
}

#endif

TEST_CASE("invert_test_00")
{
    // Some tests with non-piranha types.
    CHECK(is_invertible<float>::value);
    CHECK(is_invertible<double>::value);
    CHECK(is_invertible<long double>::value);
    CHECK(piranha::pow(1.5f, -1) ==  invert(1.5f));
    CHECK((std::is_same<decltype(std::pow(1.5f, -1)), decltype(invert(1.5f))>::value));
    CHECK(piranha::pow(1.5, -1) == invert(1.5));
    CHECK((std::is_same<double, decltype(invert(1.5))>::value));
    CHECK(piranha::pow(1.5l, -1) == invert(1.5l));
    CHECK((std::is_same<long double, decltype(invert(1.5l))>::value));
    CHECK(!is_invertible<std::string>::value);
    CHECK(!is_invertible<void>::value);
    // Test with piranha's scalar types.
    CHECK(is_invertible<integer>::value);
    CHECK(is_invertible<rational>::value);
#if defined(MPPP_WITH_MPFR)
    CHECK(is_invertible<real>::value);
#endif
    CHECK(invert(1_z) == 1);
    CHECK(invert(-1_z) == -1);
    CHECK(invert(2_z) == 0);
    CHECK((std::is_same<integer, decltype(invert(1_z))>::value));
    CHECK(invert(1_q) == 1);
    CHECK(invert(1 / 2_q) == 2);
    CHECK(invert(-2 / 3_q) == -3 / 2_q);
    CHECK((std::is_same<rational, decltype(invert(1_q))>::value));
#if defined(MPPP_WITH_MPFR)
    CHECK(invert(1_r) == 1);
    CHECK(invert(1.5_r) == piranha::pow(1.5_r, -1));
    CHECK(invert(-2.5_r) == piranha::pow(-2.5_r, -1));
    CHECK((std::is_same<real, decltype(invert(1_r))>::value));
#endif
    // Test with some series types.
    using p_type = polynomial<rational, monomial<short>>;
    CHECK(is_invertible<p_type>::value);
    p_type x{"x"};
    CHECK(invert(x) == piranha::pow(x, -1));
    CHECK((std::is_same<p_type, decltype(invert(x))>::value));
    using ps_type = poisson_series<p_type>;
    CHECK(is_invertible<ps_type>::value);
    ps_type a{"a"};
    CHECK(invert(a) == piranha::pow(a, -1));
    CHECK((std::is_same<ps_type, decltype(invert(a))>::value));
}
