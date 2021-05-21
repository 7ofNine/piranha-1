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

#include <mp++/config.hpp>

#if defined(MPPP_WITH_MPFR)

#include <piranha/real.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include <piranha/integer.hpp>
#include <piranha/is_cf.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/is_one.hpp>
#include <piranha/math/is_zero.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/rational.hpp>
#include <piranha/s11n.hpp>
#include <piranha/safe_cast.hpp>
#include <piranha/safe_convert.hpp>
#include <piranha/symbol_utils.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

static inline real operator"" _r(const char *s)
{
    return real(s, 100);
}

TEST_CASE("real_tt_test")
{
    CHECK(is_cf<real>::value);
}

TEST_CASE("real_negate_test")
{
    CHECK(has_negate<real>::value);
    real r1;
    CHECK(!r1.signbit());
    math::negate(r1);
    CHECK(r1 == 0);
    CHECK(r1.signbit());
    r1 = 123;
    math::negate(r1);
    CHECK(r1 == -123);
    math::negate(r1);
    CHECK(r1 == 123);
    r1 = real{"inf", 100};
    math::negate(r1);
    CHECK(r1 == -(real{"inf", 100}));
}

TEST_CASE("real_is_zero_test")
{
    CHECK(is_is_zero_type<real>::value);
    CHECK(is_is_zero_type<real &>::value);
    CHECK(is_is_zero_type<const real &>::value);
    CHECK(is_is_zero_type<const real>::value);
    real r1;
    CHECK(piranha::is_zero(r1));
    r1.neg();
    CHECK(piranha::is_zero(r1));
    r1 = 123;
    CHECK(!piranha::is_zero(r1));
    r1 = real{"inf", 100};
    CHECK(!piranha::is_zero(r1));
    r1 = -1;
    CHECK(!piranha::is_zero(r1));
    r1 = real{"nan", 100};
    CHECK(!piranha::is_zero(r1));
}

TEST_CASE("real_pow_test")
{
    CHECK((is_exponentiable<real, real>::value));
    CHECK((is_exponentiable<real, int>::value));
    CHECK((is_exponentiable<int, real>::value));
    CHECK((is_exponentiable<real, double>::value));
    CHECK((is_exponentiable<double, real>::value));
    CHECK((is_exponentiable<real, long double>::value));
    CHECK((is_exponentiable<long double, real>::value));
    CHECK((!is_exponentiable<void, real>::value));
    CHECK((!is_exponentiable<real, void>::value));
    CHECK((!is_exponentiable<std::string, real>::value));
    CHECK((!is_exponentiable<real, std::string>::value));
#if defined(MPPP_HAVE_GCC_INT128)
    CHECK((is_exponentiable<real, __int128_t>::value));
    CHECK((is_exponentiable<__int128_t, real>::value));
    CHECK((is_exponentiable<real, __uint128_t>::value));
    CHECK((is_exponentiable<__uint128_t, real>::value));
#endif
    {
        real r1{2}, r2{5};
        CHECK(piranha::pow(r1, r2) == 32);
        CHECK(piranha::pow(r1, 5) == 32);
        CHECK(piranha::pow(2, r2) == 32);
        CHECK(piranha::pow(r1, 5.) == 32);
        CHECK(piranha::pow(2.l, r2) == 32);
#if defined(MPPP_HAVE_GCC_INT128)
        CHECK(piranha::pow(r1, __int128_t(5)) == 32);
        CHECK(piranha::pow(__uint128_t(2), r2) == 32);
#endif
    }
    {
        // Verify perfect forwarding.
        real r0{5, 100}, r1{2, 100};
        auto res = piranha::pow(std::move(r0), r1);
        CHECK(res == 25);
        CHECK(r0.get_mpfr_t()->_mpfr_d == nullptr);
        r0 = real{5, 100};
        auto res2 = piranha::pow(r0, std::move(r1));
        CHECK(res2 == 25);
        CHECK(r1.get_mpfr_t()->_mpfr_d == nullptr);
    }
}

TEST_CASE("real_fma_test")
{
    real r0{1}, r1{4}, r2{-5};
    math::multiply_accumulate(r0, r1, r2);
    CHECK(r0 == -19);
    r0 = -5;
    r1 = -3;
    r2 = 6;
    math::multiply_accumulate(r0, r1, r2);
    CHECK(r0 == -23);
}

TEST_CASE("real_sin_cos_test")
{
    CHECK(piranha::cos(real{0, 4}) == 1);
    CHECK(piranha::sin(real{0, 4}) == 0);
    // Check stealing semantics.
    real x{1.23, 100};
    auto tmp = piranha::sin(std::move(x));
    CHECK(x.get_mpfr_t()->_mpfr_d == nullptr);
    x = real{1.23, 100};
    tmp = piranha::cos(std::move(x));
    CHECK(x.get_mpfr_t()->_mpfr_d == nullptr);
}

TEST_CASE("real_partial_test")
{
    CHECK(math::partial(real(), "") == 0);
    CHECK(math::partial(real(1), std::string("")) == 0);
    CHECK(math::partial(real(-10), std::string("")) == 0);
}

TEST_CASE("real_evaluate_test")
{
    CHECK(math::evaluate(real(), symbol_fmap<integer>{}) == real());
    CHECK(math::evaluate(real(2), symbol_fmap<int>{}) == real(2));
    CHECK(math::evaluate(real(-3.5), symbol_fmap<double>{}) == real(-3.5));
    CHECK((std::is_same<decltype(math::evaluate(real(), symbol_fmap<real>{})), real>::value));
#if defined(MPPP_HAVE_GCC_INT128)
    CHECK(math::evaluate(real(2), symbol_fmap<__int128_t>{}) == real(2));
    CHECK(math::evaluate(real(2), symbol_fmap<__uint128_t>{}) == real(2));
#endif
}

TEST_CASE("real_abs_test")
{
    CHECK(math::abs(real(42)) == real(42));
    CHECK(math::abs(real(-42)) == real(42));
    CHECK(math::abs(real("inf", 100)) == real("inf", 100));
    CHECK(math::abs(real("-inf", 100)) == real("inf", 100));
    CHECK(math::abs(real("-nan", 100)).nan_p());
}

TEST_CASE("real_safe_cast_test")
{
    CHECK((is_safely_castable<real, int>::value));
    CHECK((!is_safely_castable<real, int &>::value));
    CHECK((is_safely_castable<real &&, int>::value));
    CHECK((is_safely_castable<const real &, int>::value));
    CHECK((is_safely_castable<const real, int>::value));
    CHECK((!is_safely_castable<real, void>::value));
    CHECK((is_safely_castable<real, unsigned>::value));
    CHECK((is_safely_castable<real, integer>::value));
    CHECK((is_safely_castable<real, rational>::value));
    CHECK((is_safely_castable<real, rational>::value));
    CHECK((is_safely_castable<const real, rational>::value));
    CHECK((is_safely_castable<const real &, rational>::value));
    CHECK((!is_safely_castable<real, double>::value));
    CHECK((!is_safely_castable<real, float>::value));
    CHECK((is_safely_convertible<real, int &>::value));
    CHECK((is_safely_convertible<real &, integer &>::value));
    CHECK((is_safely_convertible<const real &, rational &>::value));
    CHECK((!is_safely_convertible<real, const int &>::value));
    CHECK((!is_safely_convertible<real, float &>::value));
    CHECK((!is_safely_convertible<real, void>::value));
    CHECK((!is_safely_castable<int, real>::value));
    CHECK((!is_safely_castable<float, real>::value));
    CHECK((!is_safely_castable<integer, real>::value));
    CHECK((!is_safely_castable<rational, real>::value));
    CHECK((!is_safely_castable<void, real>::value));
    CHECK((!is_safely_convertible<int, real &>::value));
    CHECK((!is_safely_convertible<void, real &>::value));
    CHECK(safe_cast<int>(3_r) == 3);
    CHECK(safe_cast<int>(-3_r) == -3);
    int tmp_n;
    CHECK((safe_convert(tmp_n, 3_r)));
    CHECK(tmp_n == 3);
    CHECK((!safe_convert(tmp_n, 3.12_r)));
    CHECK(tmp_n == 3);
    integer tmp_z;
    CHECK((safe_convert(tmp_z, 3_r)));
    CHECK(tmp_z == 3);
    CHECK((!safe_convert(tmp_z, 3.12_r)));
    CHECK(tmp_z == 3);
    rational tmp_q;
    CHECK((safe_convert(tmp_q, 3.5_r)));
    CHECK(tmp_q == (rational{7, 2}));
    CHECK((!safe_convert(tmp_q, real{"inf", 100})));
    CHECK(tmp_q == (rational{7, 2}));
#if defined(MPPP_HAVE_GCC_INT128)
    CHECK(safe_cast<__int128_t>(3_r) == 3);
    CHECK(safe_cast<__uint128_t>(3_r) == 3u);
    CHECK_THROWS_AS(safe_cast<__uint128_t>(-3_r), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<__uint128_t>(real{__uint128_t(-1)} * 2), safe_cast_failure);
#endif
    CHECK(safe_cast<unsigned>(4_r) == 4u);
    CHECK(safe_cast<integer>(4_r) == 4_z);
    CHECK(safe_cast<integer>(-4_r) == -4_z);
    CHECK(safe_cast<rational>(4_r) == 4_q);
    CHECK(safe_cast<rational>(-4_r) == -4_q);
    CHECK(safe_cast<rational>(5_r / 2) == 5_q / 2);
    CHECK(safe_cast<rational>(-5_r / 2) == -5_q / 2);
    // Various types of failures.
    CHECK_THROWS_MATCHES(safe_cast<int>(3.1_r), safe_cast_failure, 
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
    );
    CHECK_THROWS_AS(safe_cast<int>(-3.1_r), safe_cast_failure);
    CHECK_THROWS_MATCHES(safe_cast<int>(real{"inf", 100}), safe_cast_failure,
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
    );
    CHECK_THROWS_AS(safe_cast<int>(real{"nan", 100}), safe_cast_failure);
    CHECK_THROWS_MATCHES(
        safe_cast<int>(real{std::numeric_limits<int>::max()} * 2), safe_cast_failure,
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type")) 
    );
    CHECK_THROWS_AS(safe_cast<int>(real{std::numeric_limits<int>::min()} * 2), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<unsigned>(3.1_r), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<unsigned>(-3_r), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<unsigned>(real{"inf", 100}), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<unsigned>(real{"nan", 100}), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<unsigned>(real{std::numeric_limits<unsigned>::max()} * 2), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<integer>(3.1_r), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<integer>(-3.1_r), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<integer>(real{"inf", 100}), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<integer>(real{"nan", 100}), safe_cast_failure);
    CHECK_THROWS_AS(safe_cast<rational>(real{"inf", 100}), safe_cast_failure);
    CHECK_THROWS_MATCHES(safe_cast<rational>(real{"nan", 100}), safe_cast_failure,
        test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
    );
    // Check that real-real safe_convert() actually steals with move semantics.
    real foo1{123}, foo2{2};
    safe_convert(foo1, std::move(foo2));
    CHECK(foo1 == 2);
    // NOTE: safe_convert() will do a move assignment, which is implemented
    // as a swap() in real.
    CHECK(foo2 == real{123});
}

TEST_CASE("real_ternary_arith_test")
{
    real out;
    math::add3(out, real{4}, real{-1});
    CHECK(out == 3);
    math::sub3(out, real{4}, real{-1});
    CHECK(out == 5);
    math::mul3(out, real{4}, real{-1});
    CHECK(out == -4);
    math::div3(out, real{4}, real{-1});
    CHECK(out == -4);
}

TEST_CASE("real_is_one_test")
{
    real out;
    CHECK(!piranha::is_one(out));
    out = 1.234;
    CHECK(!piranha::is_one(out));
    out = 1;
    CHECK(piranha::is_one(out));
    out = real{"inf", 5};
    CHECK(!piranha::is_one(out));
    out = real{"-nan", 5};
    CHECK(!piranha::is_one(out));
}

#else

int main()
{
    return 0;
}

#endif
