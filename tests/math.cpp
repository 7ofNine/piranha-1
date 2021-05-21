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

#include <piranha/math.hpp>


#define FUSION_MAX_VECTOR_SIZE 20

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/sequence.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include <mp++/config.hpp>

#include <piranha/forwarding.hpp>
#include <piranha/integer.hpp>
#include <piranha/key/key_is_one.hpp>
#include <piranha/kronecker_monomial.hpp>
#include <piranha/math/cos.hpp>
#include <piranha/math/pow.hpp>
#include <piranha/math/sin.hpp>
#include <piranha/monomial.hpp>
#include <piranha/poisson_series.hpp>
#include <piranha/polynomial.hpp>
#include <piranha/rational.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/symbol_utils.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

using namespace piranha;

const boost::fusion::vector<char, short, int, long, long long, unsigned char, unsigned short, unsigned, unsigned long,
                            unsigned long long, float, double, long double>
arithmetic_values(static_cast<char>(-42), static_cast<short>(42), -42, 42L, -42LL, static_cast<unsigned char>(42),
                  static_cast<unsigned short>(42), 42U, 42UL, 42ULL, 23.456f, -23.456, 23.456L);

struct check_negate {
    template <typename T>
    void operator()(const T &value) const
    {
        if (std::is_signed<T>::value && std::is_integral<T>::value) {
            T negation(value);
            math::negate(negation);
            CHECK(negation == -value);
        }
        CHECK(has_negate<T>::value);
    }
};

struct no_negate {
};

struct no_negate2 {
    no_negate2 operator-() const;
    no_negate2(const no_negate2 &) = delete;
    no_negate2 &operator=(no_negate2 &) = delete;
};

struct yes_negate {
    yes_negate operator-() const;
};

TEST_CASE("math_negate_test")
{
    boost::fusion::for_each(arithmetic_values, check_negate());
    CHECK(!has_negate<no_negate>::value);
    CHECK(!has_negate<no_negate2>::value);
    CHECK(has_negate<yes_negate>::value);
    CHECK(has_negate<std::complex<double>>::value);
    CHECK(has_negate<int &>::value);
    CHECK(has_negate<int &&>::value);
    CHECK(!has_negate<int const>::value);
    CHECK(!has_negate<int const &>::value);
    CHECK(!has_negate<std::complex<float> const &>::value);
}

struct no_fma {
};

struct check_multiply_accumulate {
    template <typename T>
    void operator()(const T &,
                    typename std::enable_if<!std::is_same<T, char>::value && !std::is_same<T, unsigned char>::value
                                            && !std::is_same<T, signed char>::value && !std::is_same<T, short>::value
                                            && !std::is_same<T, unsigned short>::value>::type * = nullptr) const
    {
        CHECK(has_multiply_accumulate<T>::value);
        CHECK(has_multiply_accumulate<T &>::value);
        CHECK(!has_multiply_accumulate<const T>::value);
        CHECK(!has_multiply_accumulate<const T &>::value);
        T x(2);
        math::multiply_accumulate(x, T(4), T(6));
        CHECK(x == T(2) + T(4) * T(6));
        if ((std::is_signed<T>::value && std::is_integral<T>::value) || std::is_floating_point<T>::value) {
            x = T(-2);
            math::multiply_accumulate(x, T(5), T(-7));
            CHECK(x == T(-2) + T(5) * T(-7));
        }
    }
    // NOTE: avoid testing with char and short types, the promotion rules will generate warnings about assigning int to
    // narrower types.
    template <typename T>
    void operator()(const T &,
                    typename std::enable_if<std::is_same<T, char>::value || std::is_same<T, unsigned char>::value
                                            || std::is_same<T, signed char>::value || std::is_same<T, short>::value
                                            || std::is_same<T, unsigned short>::value>::type * = nullptr) const
    {
    }
};

TEST_CASE("math_multiply_accumulate_test")
{
    boost::fusion::for_each(arithmetic_values, check_multiply_accumulate());
    CHECK(!has_multiply_accumulate<no_fma>::value);
    CHECK(!has_multiply_accumulate<no_fma const &>::value);
    CHECK(!has_multiply_accumulate<no_fma &>::value);
}

TEST_CASE("math_partial_test")
{
    CHECK(piranha::is_differentiable<int>::value);
    CHECK(piranha::is_differentiable<long>::value);
    CHECK(piranha::is_differentiable<double>::value);
    CHECK(piranha::is_differentiable<double &>::value);
    CHECK(piranha::is_differentiable<double &&>::value);
    CHECK(piranha::is_differentiable<double const &>::value);
    CHECK(!piranha::is_differentiable<std::string>::value);
    CHECK(!piranha::is_differentiable<std::string &>::value);
    CHECK(math::partial(1, "") == 0);
    CHECK(math::partial(1., "") == double(0));
    CHECK(math::partial(2L, "") == 0L);
    CHECK(math::partial(2LL, std::string("")) == 0LL);
}

TEST_CASE("math_evaluate_test")
{
    CHECK(math::evaluate(5, symbol_fmap<double>{}) == 5);
    CHECK(math::evaluate(std::complex<float>(5., 4.), symbol_fmap<double>{}) == std::complex<float>(5., 4.));
    CHECK(math::evaluate(std::complex<double>(5., 4.), symbol_fmap<double>{}) ==
                      std::complex<double>(5., 4.));
    CHECK(math::evaluate(std::complex<long double>(5., 4.), symbol_fmap<double>{}) ==
                      std::complex<long double>(5., 4.));
    CHECK((std::is_same<decltype(math::evaluate(5, symbol_fmap<double>{})), double>::value));
    CHECK((std::is_same<decltype(math::evaluate(std::complex<double>(3, 5), symbol_fmap<int>{})),
                              std::complex<double>>::value));
    CHECK(math::evaluate(5., symbol_fmap<int>{}) == 5.);
    CHECK((std::is_same<decltype(math::evaluate(5., symbol_fmap<short>{})), double>::value));
    CHECK(math::evaluate(5ul, symbol_fmap<double>{}) == 5.);
    CHECK((std::is_same<decltype(math::evaluate(5ul, symbol_fmap<short>{})), unsigned long>::value));
    // Test the syntax with explicit template parameter.
    CHECK(math::evaluate<double>(5, {{"foo", 5}}) == 5);
}

TEST_CASE("math_subs_test")
{
    CHECK((!has_subs<double, double>::value));
    CHECK((!has_subs<int, double>::value));
    CHECK((!has_subs<int, char>::value));
    CHECK((!has_subs<int &, char>::value));
    CHECK((!has_subs<int, char &>::value));
    CHECK((!has_subs<int &, const char &>::value));
    CHECK((!has_subs<std::string, std::string>::value));
    CHECK((!has_subs<std::string, int>::value));
    CHECK((!has_subs<std::string &, int>::value));
    CHECK((!has_subs<int, std::string>::value));
    CHECK((!has_subs<int, std::string &&>::value));
    CHECK((!has_subs<const int, std::string &&>::value));
}

TEST_CASE("math_integrate_test")
{
    CHECK(!piranha::is_integrable<int>::value);
    CHECK(!piranha::is_integrable<int const>::value);
    CHECK(!piranha::is_integrable<int &>::value);
    CHECK(!piranha::is_integrable<int const &>::value);
    CHECK(!piranha::is_integrable<long>::value);
    CHECK(!piranha::is_integrable<double>::value);
#if defined(MPPP_WITH_MPFR)
    CHECK(!piranha::is_integrable<real>::value);
#endif
    CHECK(!piranha::is_integrable<rational>::value);
    CHECK(!piranha::is_integrable<std::string>::value);
}

TEST_CASE("math_pbracket_test")
{
    CHECK(has_pbracket<int>::value);
    CHECK(has_pbracket<double>::value);
    CHECK(has_pbracket<long double>::value);
    CHECK(!has_pbracket<std::string>::value);
    typedef polynomial<rational, monomial<int>> p_type;
    CHECK(has_pbracket<p_type>::value);
    CHECK(math::pbracket(p_type{}, p_type{}, std::vector<std::string>{}, std::vector<std::string>{}) ==
                      p_type(0));
    CHECK_THROWS_AS(math::pbracket(p_type{}, p_type{}, {"p"}, std::vector<std::string>{}), std::invalid_argument);
    CHECK_THROWS_AS(math::pbracket(p_type{}, p_type{}, {"p"}, {"q", "r"}), std::invalid_argument);
    CHECK_THROWS_AS(math::pbracket(p_type{}, p_type{}, {"p", "p"}, {"q", "r"}), std::invalid_argument);
    CHECK_THROWS_AS(math::pbracket(p_type{}, p_type{}, {"p", "q"}, {"q", "q"}), std::invalid_argument);
    CHECK(math::pbracket(p_type{}, p_type{}, {"x", "y"}, {"a", "b"}) == p_type(0));
    // Pendulum Hamiltonian.
    typedef poisson_series<polynomial<rational, monomial<int>>> ps_type;
    CHECK(has_pbracket<ps_type>::value);
    auto m = ps_type{"m"}, p = ps_type{"p"}, l = ps_type{"l"}, g = ps_type{"g"}, th = ps_type{"theta"};
    auto H_p = p * p * (2 * m * l * l).pow(-1) + m * g * l * piranha::cos(th);
    CHECK(math::pbracket(H_p, H_p, {"p"}, {"theta"}) == 0);
    // Two body problem.
    auto x = ps_type{"x"}, y = ps_type{"y"}, z = ps_type{"z"};
    auto vx = ps_type{"vx"}, vy = ps_type{"vy"}, vz = ps_type{"vz"}, r = ps_type{"r"};
    auto H_2 = (vx * vx + vy * vy + vz * vz) / 2 - r.pow(-1);
    ps_type::register_custom_derivative(
        "x", [&x, &r](const ps_type &ps) { return ps.partial("x") - ps.partial("r") * x * r.pow(-3); });
    ps_type::register_custom_derivative(
        "y", [&y, &r](const ps_type &ps) { return ps.partial("y") - ps.partial("r") * y * r.pow(-3); });
    ps_type::register_custom_derivative(
        "z", [&z, &r](const ps_type &ps) { return ps.partial("z") - ps.partial("r") * z * r.pow(-3); });
    CHECK(math::pbracket(H_2, H_2, {"vx", "vy", "vz"}, {"x", "y", "z"}) == 0);
    // Angular momentum integral.
    auto Gx = y * vz - z * vy, Gy = z * vx - x * vz, Gz = x * vy - y * vx;
    CHECK(math::pbracket(H_2, Gx, {"vx", "vy", "vz"}, {"x", "y", "z"}) == 0);
    CHECK(math::pbracket(H_2, Gy, {"vx", "vy", "vz"}, {"x", "y", "z"}) == 0);
    CHECK(math::pbracket(H_2, Gz, {"vx", "vy", "vz"}, {"x", "y", "z"}) == 0);
    CHECK(math::pbracket(H_2, Gz + x, {"vx", "vy", "vz"}, {"x", "y", "z"}) != 0);
}

TEST_CASE("math_abs_test")
{
    CHECK(has_abs<int>::value);
    CHECK(has_abs<int &>::value);
    CHECK(has_abs<const int &>::value);
    CHECK(has_abs<const int>::value);
    CHECK(has_abs<float>::value);
    CHECK(has_abs<double &&>::value);
    CHECK(!has_abs<void>::value);
    CHECK(!has_abs<std::string>::value);
    CHECK(math::abs(static_cast<signed char>(4)) == static_cast<signed char>(4));
    CHECK(math::abs(static_cast<signed char>(-4)) == static_cast<signed char>(4));
    CHECK(math::abs(short(4)) == short(4));
    CHECK(math::abs(short(-4)) == short(4));
    CHECK(math::abs(4) == 4);
    CHECK(math::abs(-4) == 4);
    CHECK(math::abs(4l) == 4l);
    CHECK(math::abs(-4l) == 4l);
    CHECK(math::abs(4ll) == 4ll);
    CHECK(math::abs(-4ll) == 4ll);
    CHECK(math::abs(static_cast<unsigned char>(4)) == static_cast<unsigned char>(4));
    CHECK(math::abs(static_cast<unsigned short>(4)) == static_cast<unsigned short>(4));
    CHECK(math::abs(4u) == 4u);
    CHECK(math::abs(4lu) == 4lu);
    ;
    CHECK(math::abs(4llu) == 4llu);
    CHECK(math::abs(1.23f) == 1.23f);
    CHECK(math::abs(-1.23f) == 1.23f);
    CHECK(math::abs(1.23) == 1.23);
    CHECK(math::abs(-1.23) == 1.23);
    CHECK(math::abs(1.23l) == 1.23l);
    CHECK(math::abs(-1.23l) == 1.23l);
}

TEST_CASE("math_has_t_degree_test")
{
    CHECK(!has_t_degree<int>::value);
    CHECK(!has_t_degree<const int>::value);
    CHECK(!has_t_degree<int &>::value);
    CHECK(!has_t_degree<const int &>::value);
    CHECK(!has_t_degree<std::string>::value);
    CHECK(!has_t_degree<double>::value);
}

TEST_CASE("math_has_t_ldegree_test")
{
    CHECK(!has_t_ldegree<int>::value);
    CHECK(!has_t_ldegree<int const>::value);
    CHECK(!has_t_ldegree<int &>::value);
    CHECK(!has_t_ldegree<const int &>::value);
    CHECK(!has_t_ldegree<std::string>::value);
    CHECK(!has_t_ldegree<double>::value);
}

TEST_CASE("math_has_t_order_test")
{
    CHECK(!has_t_order<int>::value);
    CHECK(!has_t_order<const int>::value);
    CHECK(!has_t_order<int &>::value);
    CHECK(!has_t_order<int const &>::value);
    CHECK(!has_t_order<std::string>::value);
    CHECK(!has_t_order<double>::value);
}

TEST_CASE("math_has_t_lorder_test")
{
    CHECK(!has_t_lorder<int>::value);
    CHECK(!has_t_lorder<const int>::value);
    CHECK(!has_t_lorder<int &>::value);
    CHECK(!has_t_lorder<int const &>::value);
    CHECK(!has_t_lorder<std::string>::value);
    CHECK(!has_t_lorder<double>::value);
}

TEST_CASE("math_key_has_t_degree_test")
{
    CHECK(!key_has_t_degree<monomial<int>>::value);
    CHECK(!key_has_t_degree<kronecker_monomial<>>::value);
}

TEST_CASE("math_key_has_t_ldegree_test")
{
    CHECK(!key_has_t_ldegree<monomial<int>>::value);
    CHECK(!key_has_t_ldegree<kronecker_monomial<>>::value);
}

TEST_CASE("math_key_has_t_order_test")
{
    CHECK(!key_has_t_order<monomial<int>>::value);
    CHECK(!key_has_t_order<kronecker_monomial<>>::value);
}

TEST_CASE("math_key_has_t_lorder_test")
{
    CHECK(!key_has_t_lorder<monomial<int>>::value);
    CHECK(!key_has_t_lorder<kronecker_monomial<>>::value);
}

TEST_CASE("math_t_subs_test")
{
    CHECK((!has_t_subs<double, double, double>::value));
    CHECK((!has_t_subs<int, double, double>::value));
    CHECK((!has_t_subs<int, char, char>::value));
    CHECK((!has_t_subs<int &, char, char>::value));
    CHECK((!has_t_subs<int, char &, char &>::value));
    CHECK((!has_t_subs<int, char &, short>::value));
    CHECK((!has_t_subs<int &, const char &, const char &>::value));
    CHECK((!has_t_subs<int &, const char &, const std::string &>::value));
    CHECK((!has_t_subs<std::string, std::string, std::string>::value));
    CHECK((!has_t_subs<std::string, int, int>::value));
    CHECK((!has_t_subs<std::string &, int, int>::value));
    CHECK((!has_t_subs<std::string &, int, std::string &&>::value));
}

TEST_CASE("math_canonical_test")
{
    typedef polynomial<rational, monomial<short>> p_type1;
    CHECK_THROWS_AS(math::transformation_is_canonical({p_type1{"p"}, p_type1{"p"}}, {p_type1{"q"}}, {"p"}, {"q"}),
                      std::invalid_argument);
    CHECK_THROWS_AS(math::transformation_is_canonical({p_type1{"p"}}, {p_type1{"q"}}, {"p", "x"}, {"q"}),
                      std::invalid_argument);
    CHECK_THROWS_AS(math::transformation_is_canonical({p_type1{"p"}}, {p_type1{"q"}}, {"p", "x"}, {"q", "y"}),
                      std::invalid_argument);
    CHECK((math::transformation_is_canonical({p_type1{"p"}}, {p_type1{"q"}}, {"p"}, {"q"})));
    p_type1 px{"px"}, py{"py"}, x{"x"}, y{"y"};
    CHECK((math::transformation_is_canonical({py, px}, {y, x}, {"px", "py"}, {"x", "y"})));
    CHECK((!math::transformation_is_canonical({py, px}, {x, y}, {"px", "py"}, {"x", "y"})));
    CHECK((math::transformation_is_canonical({-x, -y}, {px, py}, {"px", "py"}, {"x", "y"})));
    CHECK((!math::transformation_is_canonical({x, y}, {px, py}, {"px", "py"}, {"x", "y"})));
    CHECK((math::transformation_is_canonical({px, px + py}, {x - y, y}, {"px", "py"}, {"x", "y"})));
    CHECK((!math::transformation_is_canonical({px, px - py}, {x - y, y}, {"px", "py"}, {"x", "y"})));
    CHECK((math::transformation_is_canonical(std::vector<p_type1>{px, px + py}, std::vector<p_type1>{x - y, y},
                                                   {"px", "py"}, {"x", "y"})));
    // Linear transformation.
    p_type1 L{"L"}, G{"G"}, H{"H"}, l{"l"}, g{"g"}, h{"h"};
    CHECK((
        math::transformation_is_canonical({L + G + H, L + G, L}, {h, g - h, l - g}, {"L", "G", "H"}, {"l", "g", "h"})));
    // Unimodular matrices.
    CHECK((math::transformation_is_canonical({L + 2 * G + 3 * H, -4 * G + H, 3 * G - H},
                                                   {l, 11 * l - g - 3 * h, 14 * l - g - 4 * h}, {"L", "G", "H"},
                                                   {"l", "g", "h"})));
    CHECK((math::transformation_is_canonical(
        {2 * L + 3 * G + 2 * H, 4 * L + 2 * G + 3 * H, 9 * L + 6 * G + 7 * H},
        {-4 * l - g + 6 * h, -9 * l - 4 * g + 15 * h, 5 * l + 2 * g - 8 * h}, {"L", "G", "H"}, {"l", "g", "h"})));
    CHECK((!math::transformation_is_canonical(
        {2 * L + 3 * G + 2 * H, 4 * L + 2 * G + 3 * H, 9 * L + 6 * G + 7 * H},
        {-4 * l - g + 6 * h, -9 * l - 4 * g + 15 * h, 5 * l + 2 * g - 7 * h}, {"L", "G", "H"}, {"l", "g", "h"})));
    CHECK((math::transformation_is_canonical(
        std::vector<p_type1>{2 * L + 3 * G + 2 * H, 4 * L + 2 * G + 3 * H, 9 * L + 6 * G + 7 * H},
        std::vector<p_type1>{-4 * l - g + 6 * h, -9 * l - 4 * g + 15 * h, 5 * l + 2 * g - 8 * h}, {"L", "G", "H"},
        {"l", "g", "h"})));
    CHECK((!math::transformation_is_canonical(
        std::vector<p_type1>{2 * L + 3 * G + 2 * H, 4 * L + 2 * G + 3 * H, 9 * L + 6 * G + 7 * H},
        std::vector<p_type1>{-4 * l - g + 6 * h, -9 * l - 4 * g + 15 * h, 5 * l + 2 * g - 7 * h}, {"L", "G", "H"},
        {"l", "g", "h"})));
    typedef poisson_series<p_type1> p_type2;
    // Poincare' variables.
    p_type2 P{"P"}, Q{"Q"}, p{"p"}, q{"q"}, P2{"P2"}, Q2{"Q2"};
    p_type2::register_custom_derivative(
        "P", [&P2](const p_type2 &arg) { return arg.partial("P") + arg.partial("P2") * piranha::pow(P2, -1); });
    p_type2::register_custom_derivative(
        "Q", [&Q2](const p_type2 &arg) { return arg.partial("Q") + arg.partial("Q2") * piranha::pow(Q2, -1); });
    CHECK(
        (math::transformation_is_canonical({P2 * piranha::cos(p), Q2 * piranha::cos(q)},
                                           {P2 * piranha::sin(p), Q2 * piranha::sin(q)}, {"P", "Q"}, {"p", "q"})));
    CHECK((!math::transformation_is_canonical(
        {P * Q * piranha::cos(p) * q, Q * P * piranha::sin(3 * q) * p * piranha::pow(q, -1)},
        {P * piranha::sin(p), Q * piranha::sin(q)}, {"P", "Q"}, {"p", "q"})));
    CHECK((!math::transformation_is_canonical(
        std::vector<p_type2>{P2 * piranha::cos(p) * q, Q2 * piranha::cos(q) * p},
        std::vector<p_type2>{P2 * piranha::sin(p), Q2 * piranha::sin(q)}, {"P", "Q"}, {"p", "q"})));
    CHECK(has_transformation_is_canonical<p_type1>::value);
    CHECK(has_transformation_is_canonical<p_type1 &>::value);
    CHECK(has_transformation_is_canonical<p_type1 const &>::value);
    CHECK(has_transformation_is_canonical<p_type2>::value);
    CHECK(has_transformation_is_canonical<int>::value);
    CHECK(has_transformation_is_canonical<double>::value);
    CHECK(has_transformation_is_canonical<double &&>::value);
    CHECK(!has_transformation_is_canonical<std::string>::value);
    CHECK(!has_transformation_is_canonical<std::string &>::value);
    CHECK(!has_transformation_is_canonical<std::string const &>::value);
}

// Non-evaluable, missing copy-ctor.
struct fake_ne {
    fake_ne &operator=(const fake_ne &) = delete;
    fake_ne(const fake_ne &) = delete;
};

TEST_CASE("math_is_evaluable_test")
{
    CHECK((is_evaluable<int, int>::value));
    CHECK((is_evaluable<double, double>::value));
    CHECK((is_evaluable<double, int>::value));
    CHECK((is_evaluable<int &, int>::value));
    CHECK((is_evaluable<double, int>::value));
    CHECK((is_evaluable<double &, int>::value));
    CHECK((is_evaluable<double &&, int>::value));
    CHECK((is_evaluable<std::string, int>::value));
    CHECK((is_evaluable<std::set<int>, int>::value));
    CHECK((is_evaluable<std::string &, int>::value));
    CHECK((is_evaluable<std::set<int> &&, int>::value));
    CHECK((!is_evaluable<fake_ne, int>::value));
    CHECK((!is_evaluable<fake_ne &, int>::value));
    CHECK((!is_evaluable<fake_ne const &, int>::value));
    CHECK((!is_evaluable<fake_ne &&, int>::value));
}

namespace piranha
{
namespace math
{

// A few fake specialisations to test the degree truncation type traits.

// double-double, correct one.
template <>
struct truncate_degree_impl<double, double, void> {
    double operator()(const double &, const double &) const;
    double operator()(const double &, const double &, const symbol_fset &) const;
};

// double-float, missing one of the two overloads.
template <>
struct truncate_degree_impl<double, float, void> {
    double operator()(const double &, const float &) const;
};

// float-double, wrong return type.
template <>
struct truncate_degree_impl<float, double, void> {
    double operator()(const float &, const double &) const;
    double operator()(const float &, const double &, const symbol_fset &) const;
};
}
}

TEST_CASE("math_has_truncate_degree_test")
{
    CHECK((!has_truncate_degree<float, float>::value));
    CHECK((!has_truncate_degree<float &, float>::value));
    CHECK((!has_truncate_degree<float &, const float &>::value));
    CHECK((!has_truncate_degree<int &&, const float &>::value));
    CHECK((!has_truncate_degree<int &&, long double>::value));
    CHECK((!has_truncate_degree<std::string, long double>::value));
    CHECK((!has_truncate_degree<std::string, std::vector<int>>::value));
    CHECK((has_truncate_degree<double, double>::value));
    CHECK((has_truncate_degree<const double, double>::value));
    CHECK((has_truncate_degree<double &&, double>::value));
    CHECK((has_truncate_degree<double &&, double const &>::value));
    CHECK((!has_truncate_degree<double, float>::value));
    CHECK((!has_truncate_degree<const double, float &>::value));
    CHECK((!has_truncate_degree<double &&, float &>::value));
    CHECK((!has_truncate_degree<float, double>::value));
    CHECK((!has_truncate_degree<const float, double &>::value));
    CHECK((!has_truncate_degree<float &&, double &>::value));
}

// Mock key subs method only for certain types.
struct mock_key {
    mock_key() = default;
    mock_key(const mock_key &) = default;
    mock_key(mock_key &&) noexcept;
    mock_key &operator=(const mock_key &) = default;
    mock_key &operator=(mock_key &&) noexcept;
    mock_key(const symbol_fset &);
    bool operator==(const mock_key &) const;
    bool operator!=(const mock_key &) const;
    bool is_compatible(const symbol_fset &) const noexcept;
    mock_key merge_symbols(const symbol_idx_fmap<symbol_fset> &, const symbol_fset &) const;
    void print(std::ostream &, const symbol_fset &) const;
    void print_tex(std::ostream &, const symbol_fset &) const;
    void trim_identify(std::vector<char> &, const symbol_fset &) const;
    mock_key trim(const std::vector<char> &, const symbol_fset &) const;
    std::vector<std::pair<int, mock_key>> subs(const symbol_idx_fmap<int> &smap, const symbol_fset &args) const;
};

namespace std
{

template <>
struct hash<mock_key> {
    std::size_t operator()(const mock_key &) const;
};
}

namespace piranha
{

template <>
class key_is_one_impl<mock_key>
{
public:
    bool operator()(const mock_key &, const symbol_fset &) const;
};
}

TEST_CASE("math_key_has_subs_test")
{
    CHECK((key_has_subs<mock_key, int>::value));
    CHECK((!key_has_subs<mock_key, std::string>::value));
    CHECK((!key_has_subs<mock_key, integer>::value));
    CHECK((!key_has_subs<mock_key, rational>::value));
}

TEST_CASE("math_ternary_ops_test")
{
    {
        // Addition.
        CHECK(has_add3<int>::value);
        CHECK(has_add3<int &>::value);
        CHECK(!has_add3<const int &>::value);
        int i1 = 0;
        math::add3(i1, 3, 4);
        CHECK(i1 == 7);
        CHECK(has_add3<short>::value);
        short s1 = 1;
        math::add3(s1, short(3), short(-4));
        CHECK(s1 == -1);
        CHECK(has_add3<float>::value);
        CHECK(has_add3<double>::value);
        float f1 = 1.234f;
        math::add3(f1, 3.456f, 8.145f);
        CHECK(f1 == 3.456f + 8.145f);
        CHECK(has_add3<std::string>::value);
        std::string foo;
        math::add3(foo, std::string("hello "), std::string("world"));
        CHECK(foo == "hello world");
        CHECK(!has_add3<std::vector<int>>::value);
        CHECK(!has_add3<char *>::value);
    }
    {
        // Subtraction.
        CHECK(has_sub3<int>::value);
        CHECK(has_sub3<int &>::value);
        CHECK(!has_sub3<const int &>::value);
        int i1 = 0;
        math::sub3(i1, 3, 4);
        CHECK(i1 == -1);
        CHECK(has_sub3<short>::value);
        short s1 = 1;
        math::sub3(s1, short(3), short(-4));
        CHECK(s1 == 7);
        CHECK(has_sub3<float>::value);
        CHECK(has_sub3<double>::value);
        float f1 = 1.234f;
        math::sub3(f1, 3.456f, 8.145f);
        CHECK(f1 == 3.456f - 8.145f);
        CHECK(!has_sub3<std::string>::value);
        CHECK(!has_sub3<std::vector<int>>::value);
        CHECK(!has_sub3<char *>::value);
    }
    {
        // Multiplication.
        CHECK(has_mul3<int>::value);
        CHECK(has_mul3<int &>::value);
        CHECK(!has_mul3<const int &>::value);
        int i1 = 0;
        math::mul3(i1, 3, 4);
        CHECK(i1 == 12);
        CHECK(has_mul3<short>::value);
        short s1 = 1;
        math::mul3(s1, short(3), short(-4));
        CHECK(s1 == -12);
        CHECK(has_mul3<float>::value);
        CHECK(has_mul3<double>::value);
        float f1 = 1.234f;
        math::mul3(f1, 3.456f, 8.145f);
        CHECK(f1 == 3.456f * 8.145f);
        CHECK(!has_mul3<std::string>::value);
        CHECK(!has_mul3<std::vector<int>>::value);
        CHECK(!has_mul3<char *>::value);
    }
    {
        // Division.
        CHECK(has_div3<int>::value);
        CHECK(has_div3<int &>::value);
        CHECK(!has_div3<const int &>::value);
        int i1 = 0;
        math::div3(i1, 6, 3);
        CHECK(i1 == 2);
        CHECK(has_div3<short>::value);
        short s1 = -8;
        math::div3(s1, short(-8), short(2));
        CHECK(s1 == -4);
        CHECK(has_div3<float>::value);
        CHECK(has_div3<double>::value);
        float f1 = 1.234f;
        math::div3(f1, 3.456f, 8.145f);
        CHECK(f1 == 3.456f / 8.145f);
        CHECK(!has_div3<std::string>::value);
        CHECK(!has_div3<std::vector<int>>::value);
        CHECK(!has_div3<char *>::value);
    }
}
