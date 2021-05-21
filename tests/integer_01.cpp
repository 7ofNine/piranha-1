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

#include <piranha/integer.hpp>


#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>

#include <boost/algorithm/string/predicate.hpp>

#include <mp++/config.hpp>
#include <mp++/exceptions.hpp>
#include <mp++/integer.hpp>

#include <piranha/config.hpp>
#include <piranha/math.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/gcd.hpp>
#include <piranha/math/gcd3.hpp>
#include <piranha/math/is_one.hpp>
#include <piranha/math/is_zero.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/safe_cast.hpp>
#include <piranha/safe_convert.hpp>
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

using size_types = std::tuple<std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 2>,
                              std::integral_constant<std::size_t, 3>, std::integral_constant<std::size_t, 7>,
                              std::integral_constant<std::size_t, 10>>;

struct negate_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(has_negate<int_type>::value);
        CHECK(has_negate<int_type &>::value);
        CHECK(!has_negate<const int_type &>::value);
        CHECK(!has_negate<const int_type>::value);
        int_type n;
        math::negate(n);
        CHECK(n == 0);
        n = 4;
        math::negate(n);
        CHECK(n == -4);
        n.promote();
        math::negate(n);
        CHECK(n == 4);
    }
};

TEST_CASE("integer_negate_test")
{
    tuple_for_each(size_types{}, negate_tester{});
}

struct is_zero_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(is_is_zero_type<int_type>::value);
        CHECK(is_is_zero_type<const int_type>::value);
        CHECK(is_is_zero_type<int_type &>::value);
        CHECK(is_is_zero_type<const int_type &>::value);
        int_type n;
        CHECK(piranha::is_zero(n));
        n = 1;
        CHECK(!piranha::is_zero(n));
        n = 101;
        CHECK(!piranha::is_zero(n));
        n = -1;
        CHECK(!piranha::is_zero(n));
        n = -101;
        CHECK(!piranha::is_zero(n));
        n = 0;
        n.promote();
        CHECK(piranha::is_zero(n));
        n = 1;
        CHECK(!piranha::is_zero(n));
        n = 101;
        CHECK(!piranha::is_zero(n));
        n = -1;
        CHECK(!piranha::is_zero(n));
        n = -101;
        CHECK(!piranha::is_zero(n));
    }
};

TEST_CASE("integer_is_zero_test")
{
    tuple_for_each(size_types{}, is_zero_tester{});
}

struct addmul_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(has_multiply_accumulate<int_type>::value);
        CHECK(has_multiply_accumulate<int_type &>::value);
        CHECK(!has_multiply_accumulate<const int_type &>::value);
        CHECK(!has_multiply_accumulate<const int_type>::value);
        int_type a{1}, b{2}, c{3};
        math::multiply_accumulate(a, b, c);
        CHECK(a == 7);
        b.promote();
        c = -5;
        math::multiply_accumulate(a, b, c);
        CHECK(a == -3);
    }
};

TEST_CASE("integer_multiply_accumulate_test")
{
    tuple_for_each(size_types{}, addmul_tester{});
}

struct is_one_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(is_is_one_type<int_type>::value);
        CHECK(is_is_one_type<const int_type>::value);
        CHECK(is_is_one_type<int_type &>::value);
        CHECK(is_is_one_type<const int_type &>::value);
        int_type n;
        CHECK(!piranha::is_one(n));
        n = 1;
        CHECK(piranha::is_one(n));
        n = -1;
        CHECK(!piranha::is_one(n));
        n.promote();
        CHECK(!piranha::is_one(n));
        n = 1;
        n.promote();
        CHECK(piranha::is_one(n));
    }
};

TEST_CASE("integer_is_one_test")
{
    tuple_for_each(size_types{}, is_one_tester{});
}

struct abs_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(has_abs<int_type>::value);
        CHECK(has_abs<const int_type>::value);
        CHECK(has_abs<int_type &>::value);
        CHECK(has_abs<const int_type &>::value);
        int_type n;
        CHECK(math::abs(n) == 0);
        n = -1;
        CHECK(math::abs(n) == 1);
        n = 123;
        n.promote();
        CHECK(math::abs(n) == 123);
    }
};

TEST_CASE("integer_abs_test")
{
    tuple_for_each(size_types{}, abs_tester{});
}

struct sin_cos_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(piranha::sin(int_type()) == 0);
        CHECK(piranha::cos(int_type()) == 1);
        CHECK_THROWS_MATCHES(piranha::sin(int_type(1)), std::domain_error, test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the sine of the non-zero integer 1"))
        );
        CHECK_THROWS_MATCHES(piranha::cos(int_type(1)), std::domain_error, test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the cosine of the non-zero integer 1"))
        );
        CHECK((std::is_same<int_type, decltype(piranha::cos(int_type{}))>::value));
        CHECK((std::is_same<int_type, decltype(piranha::sin(int_type{}))>::value));
        CHECK(is_sine_type<int_type>::value);
        CHECK(is_cosine_type<int_type>::value);
        CHECK(is_sine_type<int_type &>::value);
        CHECK(is_cosine_type<int_type &>::value);
        CHECK(is_sine_type<const int_type &>::value);
        CHECK(is_cosine_type<const int_type &>::value);
    }
};

TEST_CASE("integer_sin_cos_test")
{
    tuple_for_each(size_types{}, sin_cos_tester{});
}

struct partial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(is_differentiable<int_type>::value);
        CHECK(is_differentiable<int_type &>::value);
        CHECK(is_differentiable<const int_type &>::value);
        CHECK(is_differentiable<const int_type>::value);
        int_type n;
        CHECK(math::partial(n, "") == 0);
        n = 5;
        CHECK(math::partial(n, "abc") == 0);
        n = -5;
        CHECK(math::partial(n, "def") == 0);
    }
};

TEST_CASE("integer_partial_test")
{
    tuple_for_each(size_types{}, partial_tester{});
}

struct factorial_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        int_type n;
        CHECK(math::factorial(n) == 1);
        n = 1;
        CHECK(math::factorial(n) == 1);
        n = 2;
        CHECK(math::factorial(n) == 2);
        n = 3;
        CHECK(math::factorial(n) == 6);
        n = 4;
        CHECK(math::factorial(n) == 24);
        n = 5;
        CHECK(math::factorial(n) == 24 * 5);
        CHECK_THROWS_MATCHES(math::factorial(int_type{-1}), std::domain_error, test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the factorial of the negative integer -1"))
        );
        CHECK_THROWS_MATCHES(math::factorial(int_type{-10}), std::domain_error, test::ExceptionMatcher<std::domain_error>(std::string("cannot compute the factorial of the negative integer -10"))
        );
        n = std::numeric_limits<unsigned long>::max();
        ++n;
        CHECK_THROWS_AS(math::factorial(n), std::overflow_error);
        n = 1000001ull;
        CHECK_THROWS_AS(math::factorial(n), std::invalid_argument);
    }
};

TEST_CASE("integer_factorial_test")
{
    tuple_for_each(size_types{}, factorial_tester{});
}

struct b_00 {
    b_00() = default;
    b_00(const b_00 &) = delete;
    b_00(b_00 &&) = delete;
};

struct b_01 {
    b_01() = default;
    b_01(const b_01 &) = default;
    b_01(b_01 &&) = default;
    ~b_01() = delete;
};

namespace piranha
{

namespace math
{

template <>
struct ipow_subs_impl<b_00, b_00> {
    b_00 operator()(const b_00 &, const std::string &, const integer &, const b_00 &) const;
};

template <>
struct ipow_subs_impl<b_01, b_01> {
    b_01 operator()(const b_01 &, const std::string &, const integer &, const b_01 &) const;
};
}
}

struct ipow_subs_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK((!has_ipow_subs<int_type, int_type>::value));
        CHECK((!has_ipow_subs<int_type, int>::value));
        CHECK((!has_ipow_subs<int_type, long>::value));
        CHECK((!has_ipow_subs<int_type, double>::value));
        CHECK((!has_ipow_subs<int_type &, int_type>::value));
        CHECK((!has_ipow_subs<int_type, const int>::value));
        CHECK((!has_ipow_subs<int_type, void>::value));
        CHECK((!has_ipow_subs<const int_type &, double>::value));
        CHECK((!has_ipow_subs<void, void>::value));
    }
};

TEST_CASE("integer_ipow_subs_test")
{
    tuple_for_each(size_types{}, ipow_subs_tester{});
    CHECK((!has_ipow_subs<b_00, b_00>::value));
    CHECK((!has_ipow_subs<b_01, b_01>::value));
}

struct ternary_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(has_add3<int_type>::value);
        CHECK(has_add3<int_type &>::value);
        CHECK(!has_add3<const int_type &>::value);
        CHECK(!has_add3<const int_type>::value);
        CHECK(has_sub3<int_type>::value);
        CHECK(has_sub3<int_type &>::value);
        CHECK(!has_sub3<const int_type &>::value);
        CHECK(!has_sub3<const int_type>::value);
        CHECK(has_mul3<int_type>::value);
        CHECK(has_mul3<int_type &>::value);
        CHECK(!has_mul3<const int_type &>::value);
        CHECK(!has_mul3<const int_type>::value);
        CHECK(has_div3<int_type>::value);
        CHECK(has_div3<int_type &>::value);
        CHECK(!has_div3<const int_type &>::value);
        CHECK(!has_div3<const int_type>::value);
        int_type a, b{1}, c{-3};
        math::add3(a, b, c);
        CHECK(a == -2);
        math::sub3(a, b, c);
        CHECK(a == 4);
        math::mul3(a, b, c);
        CHECK(a == -3);
        b = 6;
        c = -2;
        math::div3(a, b, c);
        CHECK(a == -3);
        c = 0;
        CHECK_THROWS_AS(math::div3(a, b, c), mppp::zero_division_error);
    }
};

TEST_CASE("integer_ternary_test")
{
    tuple_for_each(size_types{}, ternary_tester{});
}

struct gcd_tester {
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK((are_gcd_types<int_type>::value));
        CHECK((are_gcd_types<int_type, int>::value));
        CHECK((are_gcd_types<long, int_type>::value));
        CHECK((are_gcd_types<char &, const int_type>::value));
        CHECK((are_gcd_types<const char &, const int_type>::value));
        CHECK((are_gcd3_types<int_type &, int_type>::value));
        CHECK((are_gcd3_types<int_type &, int_type &&, const int_type &>::value));
        CHECK((are_gcd3_types<int_type &, int, long>::value));
        CHECK((!are_gcd3_types<int_type, int_type, int_type>::value));
        CHECK((!are_gcd3_types<const int_type &, int_type &>::value));
        CHECK((!are_gcd3_types<const int_type, int_type const>::value));
        CHECK((are_gcd_types<int_type, wchar_t>::value));
        CHECK((are_gcd_types<wchar_t, int_type>::value));
        CHECK((!are_gcd_types<int_type, void>::value));
        CHECK((!are_gcd_types<void, int_type>::value));
#if defined(MPPP_HAVE_GCC_INT128)
        CHECK((are_gcd_types<int_type, __int128_t>::value));
        CHECK((are_gcd_types<__int128_t, int_type>::value));
        CHECK((are_gcd_types<int_type, __uint128_t>::value));
        CHECK((are_gcd_types<__uint128_t, int_type>::value));
#endif
        CHECK(piranha::gcd(int_type{4}, int_type{6}) == 2);
        CHECK(piranha::gcd(int_type{0}, int_type{-6}) == 6);
        CHECK(piranha::gcd(int_type{6}, int_type{0}) == 6);
        CHECK(piranha::gcd(int_type{0}, int_type{0}) == 0);
        CHECK(piranha::gcd(-4, int_type{6}) == 2);
        CHECK(piranha::gcd(int_type{4}, -6ll) == 2);
#if defined(MPPP_HAVE_GCC_INT128)
        CHECK(piranha::gcd(__int128_t(-4), int_type{6}) == 2);
        CHECK(piranha::gcd(int_type{4}, __uint128_t(6)) == 2);
#endif
        int_type n;
        piranha::gcd3(n, int_type{4}, int_type{6});
        CHECK(n == 2);
        piranha::gcd3(n, -4, int_type{6});
        CHECK(n == 2);
        piranha::gcd3(n, int_type{-4}, 6);
        CHECK(n == 2);
        piranha::gcd3(n, 4, -6);
        CHECK(n == 2);
        piranha::gcd3(n, int_type{0}, int_type{0});
        CHECK(n == 0);
    }
};

TEST_CASE("integer_gcd_test")
{
    tuple_for_each(size_types{}, gcd_tester{});
    CHECK((!are_gcd_types<mppp::integer<1>, mppp::integer<2>>::value));
    CHECK((!are_gcd_types<mppp::integer<2>, mppp::integer<1>>::value));
}

TEST_CASE("integer_literal_test")
{
    auto n0 = 12345_z;
    CHECK((std::is_same<integer, decltype(n0)>::value));
    CHECK(n0 == 12345);
    n0 = -456_z;
    CHECK(n0 == -456l);
    CHECK_THROWS_AS((n0 = -1234.5_z), std::invalid_argument);
    CHECK(n0 == -456l);
}

using fp_types = std::tuple<float, double
#if defined(MPPP_WITH_MPFR)
                            ,
                            long double
#endif
                            >;

using int_types = std::tuple<char, signed char, unsigned char, short, unsigned short, int, unsigned, long,
                             unsigned long, long long, unsigned long long>;

struct safe_cast_float_tester {
    template <typename S>
    struct runner {
        template <typename T>
        void operator()(const T &) const
        {
            using int_type = mppp::integer<S::value>;
            // Type traits.
            CHECK((is_safely_convertible<const T &, int_type &>::value));
            CHECK((!is_safely_convertible<const T &, const int_type &>::value));
            CHECK((!is_safely_convertible<const T &, int_type &&>::value));
            CHECK((!is_safely_convertible<void, int_type &>::value));
            CHECK((is_safely_castable<T, int_type>::value));
            CHECK((!is_safely_castable<T, int_type &>::value));
            CHECK((!is_safely_castable<T, const int_type>::value));
            CHECK((is_safely_castable<T &, int_type>::value));
            CHECK((is_safely_castable<const T &, int_type>::value));
            CHECK((is_safely_castable<T &&, int_type>::value));
            CHECK((!is_safely_castable<void, int_type>::value));
            CHECK((!is_safely_castable<int_type, void>::value));

            // Simple testing.
            int_type tmp_n;
            CHECK(safe_convert(tmp_n, T(123)));
            CHECK(tmp_n == 123);
            tmp_n = 0;
            CHECK(!safe_convert(tmp_n, T(123.12)));
            CHECK(tmp_n == 0);
            CHECK(safe_cast<int_type>(T(0)) == int_type{0});
            CHECK(safe_cast<int_type>(T(-1)) == int_type{-1});
            CHECK(safe_cast<int_type>(T(1)) == int_type{1});
            CHECK_THROWS_MATCHES(safe_cast<int_type>(T(1.5)), safe_cast_failure, test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
            );
            CHECK_THROWS_MATCHES(safe_cast<int_type>(T(-1.5)), safe_cast_failure, test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
            );

            // Non-finite values.
            using lim = std::numeric_limits<T>;
            if (lim::is_iec559) {
                CHECK_THROWS_MATCHES(safe_cast<int_type>(lim::quiet_NaN()), safe_cast_failure,
                                      test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
                );
                CHECK_THROWS_MATCHES(safe_cast<int_type>(lim::infinity()), safe_cast_failure,
                                      test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
                );
            }
        }
    };
    template <typename S>
    void operator()(const S &) const
    {
        tuple_for_each(fp_types{}, runner<S>{});
    }
};

TEST_CASE("integer_safe_cast_float_test")
{
    tuple_for_each(size_types{}, safe_cast_float_tester());
}

struct safe_cast_int_tester {
    template <typename S>
    struct runner {
        template <typename T>
        void operator()(const T &) const
        {
            using int_type = mppp::integer<S::value>;
            // Type trait.
            CHECK((is_safely_convertible<T, int_type &>::value));
            CHECK((!is_safely_convertible<T, const int_type &>::value));
            CHECK((!is_safely_convertible<T, int_type &&>::value));
            CHECK((!is_safely_convertible<void, int_type &&>::value));
            CHECK((is_safely_convertible<int_type, T &>::value));
            CHECK((!is_safely_convertible<int_type, const T &>::value));
            CHECK((!is_safely_convertible<int_type, T &&>::value));
            CHECK((!is_safely_convertible<void, T &&>::value));
            CHECK((is_safely_castable<T, int_type>::value));
            CHECK((is_safely_castable<T &, int_type>::value));
            CHECK((is_safely_castable<const T &, int_type>::value));
            CHECK((is_safely_castable<T &&, int_type>::value));
            CHECK((!is_safely_castable<T &&, int_type &>::value));
            CHECK((!is_safely_castable<T &&, const int_type>::value));
            CHECK((is_safely_castable<int_type, T>::value));
            CHECK((is_safely_castable<int_type &, T>::value));
            CHECK((is_safely_castable<const int_type, T>::value));
            CHECK((is_safely_castable<const int_type &, T>::value));
            CHECK((!is_safely_castable<const int_type, T &>::value));
            CHECK((!is_safely_castable<const int_type, const T &>::value));

            // Simple checks.
            int_type tmp_n;
            CHECK(safe_convert(tmp_n, T(12)));
            CHECK(tmp_n == 12);
            T tmp_m;
            CHECK(safe_convert(tmp_m, int_type(12)));
            CHECK(tmp_m == T(12));
            CHECK(safe_cast<int_type>(T(0)) == int_type{0});
            CHECK(safe_cast<int_type>(T(1)) == int_type{1});
            CHECK(safe_cast<int_type>(T(12)) == int_type{12});
            CHECK(safe_cast<T>(int_type(0)) == T{0});
            CHECK(safe_cast<T>(int_type(1)) == T{1});
            CHECK(safe_cast<T>(int_type{12}) == T{12});

            // Failures.
            using lim = std::numeric_limits<T>;
            CHECK_THROWS_MATCHES(safe_cast<T>(int_type(lim::max()) + 1), safe_cast_failure,
                                 test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
            );
            CHECK_THROWS_MATCHES(safe_cast<T>(int_type(lim::min()) - 1), safe_cast_failure,
                                 test::ExceptionMatcher<safe_cast_failure>(std::string("the safe conversion of a value of type"))
            );
        }
    };
    template <typename S>
    void operator()(const S &) const
    {
        tuple_for_each(int_types{}, runner<S>{});
        using int_type = mppp::integer<S::value>;
        CHECK((is_safely_castable<wchar_t, int_type>::value));
        // NOTE: further demangler fixes are needed in mp++, the problem
        // here is that we will be trying to demangle references to 128bit
        // integers rather than pure 128bit ints. Let's disable for now.
#if defined(MPPP_HAVE_GCC_INT128) && !defined(__apple_build_version__)
        CHECK((is_safely_castable<__int128_t, int_type>::value));
        CHECK((is_safely_castable<__uint128_t, int_type>::value));
        CHECK((is_safely_castable<int_type, __int128_t>::value));
        CHECK((is_safely_castable<int_type, __uint128_t>::value));
        CHECK(safe_cast<__int128_t>(int_type{12}) == 12);
        CHECK(safe_cast<__uint128_t>(int_type{12}) == 12u);
        CHECK(safe_cast<int_type>(__int128_t(12)) == 12);
        CHECK(safe_cast<int_type>(__uint128_t(12)) == 12);
#endif
    }
};

TEST_CASE("integer_safe_cast_int_test")
{
    tuple_for_each(size_types{}, safe_cast_int_tester{});
}

struct sep_tester {
    template <typename T>
    using edict = symbol_fmap<T>;
    template <typename T>
    void operator()(const T &) const
    {
        using int_type = mppp::integer<T::value>;
        CHECK(math::evaluate(int_type{12}, edict<int>{{"", 1}}) == 12);
        CHECK(math::evaluate(int_type{10}, edict<double>{{"", 1.321}}) == 10);
        CHECK((is_evaluable<int_type, int>::value));
        CHECK((is_evaluable<int_type, double>::value));
#if defined(MPPP_WITH_MPFR)
        CHECK((std::is_same<long double,
                                  decltype(math::evaluate(int_type{10}, edict<long double>{{"", 1.321l}}))>::value));
#else
        CHECK(
            (std::is_same<int_type, decltype(math::evaluate(int_type{10}, edict<long double>{{"", 1.321l}}))>::value));
#endif
#if defined(MPPP_HAVE_GCC_INT128)
        CHECK((is_evaluable<int_type, __int128_t>::value));
        CHECK((is_evaluable<int_type, __uint128_t>::value));
        CHECK((math::evaluate(int_type{12}, edict<__int128_t>{{"", 1}}) == 12));
        CHECK((math::evaluate(int_type{12}, edict<__uint128_t>{{"", 1}}) == 12));
#endif
        CHECK((std::is_same<double, decltype(math::evaluate(int_type{10}, edict<double>{{"", 1.321}}))>::value));
        CHECK((!has_subs<int_type, int_type>::value));
        CHECK((!has_subs<int_type, int>::value));
        CHECK((!has_subs<int_type, long double>::value));
        CHECK((!has_ipow_subs<int_type, int>::value));
        CHECK((!has_ipow_subs<int_type, double>::value));
        CHECK((!has_ipow_subs<int_type, float>::value));
        CHECK((!has_ipow_subs<int_type, unsigned short>::value));
    }
};

TEST_CASE("integer_sep_test")
{
    tuple_for_each(size_types{}, sep_tester());
}
