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

#include <piranha/small_vector.hpp>

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include <piranha/config.hpp>
#include <piranha/detail/prepare_for_print.hpp>
#include <piranha/exceptions.hpp>
#include <piranha/integer.hpp>
#include <piranha/rational.hpp>
#include <piranha/s11n.hpp>
#include <piranha/safe_cast.hpp>
#include <piranha/safe_convert.hpp>

#include "catch.hpp"

// NOTE: in these tests we are assuming a few things:
// - we can generally go a few elements beyond the numerical limits of sizes without wrapping over,
// - the static size will be less than the dynamic size,
// - we can interoperate safely with the size_type of std::vector.
// These seem pretty safe in any concievable situation, but just keep it in mind. Note that the implementation
// does not care about these assumptions, it's just the tests that do.

#if defined(PIRANHA_WITH_BOOST_S11N)
static const int ntries = 1000;
#endif

static std::mt19937 rng;

using namespace piranha;

typedef boost::mpl::vector<signed char, short, int, long, long long /* integer, rational*/> value_types;  //integer and  rational don't have an input operator >> and thus fails lexical_cast
typedef boost::mpl::vector<std::integral_constant<std::size_t, 0u>, std::integral_constant<std::size_t, 1u>,
                           std::integral_constant<std::size_t, 5u>, std::integral_constant<std::size_t, 10u>>
    size_types;

// Class that throws after a few copies.
struct time_bomb {
    time_bomb() : m_vector(5) {}
    time_bomb(time_bomb &&) = default;
    time_bomb(const time_bomb &other) : m_vector(other.m_vector)
    {
        if (s_counter == 2u) {
            throw std::runtime_error("ka-pow!");
        }
        ++s_counter;
    }
    time_bomb &operator=(const time_bomb &) = default;
    time_bomb &operator=(time_bomb &&other) noexcept
    {
        m_vector = std::move(other.m_vector);
        return *this;
    }
    ~time_bomb() noexcept {}
    std::vector<int> m_vector;
    static unsigned s_counter;
};

unsigned time_bomb::s_counter = 0u;

struct dynamic_tester {
    template <typename T>
    void operator()(const T &)
    {
        typedef detail::dynamic_storage<T> d1;
        CHECK(is_container_element<d1>::value);
        d1 ds1;
        CHECK(ds1.begin() == ds1.end());
        CHECK(static_cast<const d1 &>(ds1).begin() == static_cast<const d1 &>(ds1).end());
        CHECK(static_cast<const d1 &>(ds1).begin() == ds1.end());
        CHECK(ds1.begin() == static_cast<const d1 &>(ds1).end());
        CHECK(ds1.empty());
        CHECK(ds1.size() == 0u);
        CHECK(ds1.capacity() == 0u);
        d1 ds2(ds1);
        CHECK(ds2.size() == 0u);
        CHECK(ds2.capacity() == 0u);
        ds1.push_back(T(0));
        CHECK(ds1[0u] == T(0));
        CHECK(ds1.capacity() == 1u);
        CHECK(ds1.size() == 1u);
        CHECK(!ds1.empty());
        d1 ds3(ds1);
        CHECK(ds3[0u] == T(0));
        CHECK(ds3.capacity() == 1u);
        CHECK(ds3.size() == 1u);
        d1 ds4(std::move(ds3));
        CHECK(ds4[0u] == T(0));
        CHECK(ds4.capacity() == 1u);
        CHECK(ds4.size() == 1u);
        CHECK(ds3.capacity() == 0u);
        CHECK(ds3.size() == 0u);
        d1 ds5(ds2);
        CHECK(ds5.size() == 0u);
        CHECK(ds5.capacity() == 0u);
        T tmp(1);
        ds1.push_back(tmp);
        CHECK(ds1[1u] == T(1));
        CHECK(ds1.capacity() == 2u);
        CHECK(ds1.size() == 2u);
        ds1.reserve(1u);
        CHECK(ds1[0u] == T(0));
        CHECK(ds1[1u] == T(1));
        CHECK(ds1.capacity() == 2u);
        CHECK(ds1.size() == 2u);
        d1 ds6(std::move(ds1));
        CHECK(ds6[0u] == T(0));
        CHECK(ds6[1u] == T(1));
        CHECK(ds6.capacity() == 2u);
        CHECK(ds6.size() == 2u);
        d1 ds7;
        ds7.reserve(10u);
        CHECK(ds7.capacity() == 10u);
        CHECK(ds7.size() == 0u);
        for (int i = 0; i < 11; ++i) {
            ds7.push_back(T(i));
        }
        CHECK(ds7.capacity() == 20u);
        CHECK(ds7.size() == 11u);
        std::vector<T> tmp_vec = {T(0), T(1), T(2), T(3), T(4), T(5), T(6), T(7), T(8), T(9), T(10)};
        CHECK(std::equal(tmp_vec.begin(), tmp_vec.end(), ds7.begin()));
        d1 ds8;
        std::vector<T> tmp_vec2;
        for (typename d1::size_type i = 0; i < d1::max_size; ++i) {
            ds8.push_back(T(i));
            tmp_vec2.push_back(T(i));
        }
        CHECK(std::equal(ds8.begin(), ds8.end(), tmp_vec2.begin()));
        CHECK_THROWS_AS(ds8.push_back(T(0)), std::bad_alloc);
        d1 ds9;
        ds9.reserve(d1::max_size - 1u);
        for (typename d1::size_type i = 0; i < d1::max_size; ++i) {
            ds9.push_back(T(i));
        }
        CHECK(std::equal(ds9.begin(), ds9.end(), ds8.begin()));
        detail::dynamic_storage<time_bomb> ds10;
        ds10.push_back(time_bomb{});
        ds10.push_back(time_bomb{});
        ds10.push_back(time_bomb{});
        ds10.push_back(time_bomb{});
        CHECK_THROWS_AS((detail::dynamic_storage<time_bomb>{ds10}), std::runtime_error);
        // Assignment.
        d1 ds11, ds12;
        ds11.push_back(T(42));
        auto ptr1 = &ds11[0u];
        ds11 = ds11;
        ds11 = std::move(ds11);
        CHECK(ptr1 == &ds11[0u]);
        CHECK(ds11.size() == 1u);
        CHECK(ds11.capacity() == 1u);
        CHECK(ds11[0u] == T(42));
        ds12 = std::move(ds11);
        CHECK(ds12.size() == 1u);
        CHECK(ds12.capacity() == 1u);
        CHECK(ds12[0u] == T(42));
        CHECK(ds11.size() == 0u);
        CHECK(ds11.capacity() == 0u);
        // Revive with assignment.
        ds11 = ds12;
        CHECK(ds11.size() == 1u);
        CHECK(ds11.capacity() == 1u);
        CHECK(ds11[0u] == T(42));
        auto ds13 = std::move(ds11);
        ds11 = std::move(ds13);
        CHECK(ds11.size() == 1u);
        CHECK(ds11.capacity() == 1u);
        CHECK(ds11[0u] == T(42));
        ds11.push_back(T(43));
        ds11.push_back(T(44));
        ds11.push_back(T(45));
        CHECK(ds11.size() == 4u);
        CHECK(ds11.capacity() == 4u);
        // Iterators tests.
        auto it1 = ds11.begin();
        std::advance(it1, 4);
        CHECK(it1 == ds11.end());
        auto it2 = static_cast<d1 const &>(ds11).begin();
        std::advance(it2, 4);
        CHECK(it2 == static_cast<d1 const &>(ds11).end());
        CHECK(ds11.begin() == &ds11[0u]);
        // Some STL algos.
        d1 ds14;
        std::copy(tmp_vec.rbegin(), tmp_vec.rend(), std::back_inserter(ds14));
        std::shuffle(ds14.begin(), ds14.end(), std::random_device());
        std::stable_sort(ds14.begin(), ds14.end());
        CHECK(*std::max_element(ds14.begin(), ds14.end()) == T(10));
        CHECK(*std::min_element(ds14.begin(), ds14.end()) == T(0));
        CHECK(std::equal(ds14.begin(), ds14.end(), tmp_vec.begin()));
        // Capacity tests.
        const auto orig_cap = ds14.capacity();
        const auto orig_ptr = ds14[0u];
        ds14.reserve(0u);
        CHECK(ds14.capacity() == orig_cap);
        CHECK(orig_ptr == ds14[0u]);
        ds14.reserve(orig_cap);
        CHECK(ds14.capacity() == orig_cap);
        CHECK(orig_ptr == ds14[0u]);
        // Hash.
        d1 ds15;
        CHECK(ds15.hash() == 0u);
        ds15.push_back(T(1));
        CHECK(ds15.hash() == std::hash<T>()(T(1)));
        // Resizing.
        auto ptr = &ds15[0u];
        ds15.resize(1u);
        CHECK(ds15.size() == 1u);
        CHECK(ds15.capacity() == 1u);
        CHECK(&ds15[0u] == ptr);
        ds15.resize(0u);
        CHECK(ds15.size() == 0u);
        CHECK(ds15.capacity() == 1u);
        ds15.resize(0u);
        CHECK(ds15.size() == 0u);
        CHECK(ds15.capacity() == 1u);
        ds15.resize(100u);
        CHECK(std::equal(ds15.begin(), ds15.end(), std::vector<T>(100u).begin()));
        ds15.resize(200u);
        CHECK(std::equal(ds15.begin(), ds15.end(), std::vector<T>(200u).begin()));
        ds15.resize(199u);
        CHECK(std::equal(ds15.begin(), ds15.end(), std::vector<T>(199u).begin()));
        d1 ds16;
        std::vector<T> cmp;
        int n = 0;
        std::generate_n(std::back_inserter(cmp), 100, [&n]() { return T(n++); });
        n = 0;
        std::generate_n(std::back_inserter(ds16), 100, [&n]() { return T(n++); });
        CHECK(std::equal(ds16.begin(), ds16.end(), cmp.begin()));
        ptr = &ds16[0u];
        ds16.resize(101);
        CHECK(ptr == &ds16[0u]);
        cmp.resize(101);
        CHECK(std::equal(ds16.begin(), ds16.end(), cmp.begin()));
        ds16.resize(100);
        CHECK(ptr == &ds16[0u]);
        cmp.resize(100);
        CHECK(std::equal(ds16.begin(), ds16.end(), cmp.begin()));
        auto old_cap = ds16.capacity();
        ds16.resize(129);
        cmp.resize(129);
        CHECK(std::equal(ds16.begin(), ds16.end(), cmp.begin()));
        CHECK(old_cap != ds16.capacity());
        old_cap = ds16.capacity();
        ptr = &ds16[0];
        ds16.resize(1);
        cmp.resize(1);
        CHECK(ptr == &ds16[0]);
        CHECK(cmp[0] == ds16[0]);
        ds16.resize(1);
        CHECK(ptr == &ds16[0]);
        ds16.resize(0);
        CHECK(old_cap == ds16.capacity());
        {
            // Erase testing.
            typedef detail::dynamic_storage<T> vector_type;
            vector_type v1;
            v1.push_back(boost::lexical_cast<T>(1));
            auto it = v1.erase(v1.begin());
            CHECK(v1.empty());
            CHECK(it == v1.end());
            v1.push_back(boost::lexical_cast<T>(1));
            v1.push_back(boost::lexical_cast<T>(2));
            it = v1.erase(v1.begin());
            CHECK(v1.size() == 1u);
            CHECK(it == v1.begin());
            CHECK(v1[0u] == boost::lexical_cast<T>(2));
            it = v1.erase(v1.begin());
            CHECK(v1.empty());
            CHECK(it == v1.end());
            v1.push_back(boost::lexical_cast<T>(1));
            v1.push_back(boost::lexical_cast<T>(2));
            it = v1.erase(v1.begin() + 1);
            CHECK(v1.size() == 1u);
            CHECK(it == v1.end());
            CHECK(v1[0u] == boost::lexical_cast<T>(1));
            it = v1.erase(v1.begin());
            CHECK(v1.empty());
            CHECK(it == v1.end());
            v1.push_back(boost::lexical_cast<T>(1));
            v1.push_back(boost::lexical_cast<T>(2));
            v1.push_back(boost::lexical_cast<T>(3));
            v1.push_back(boost::lexical_cast<T>(4));
            it = v1.erase(v1.begin());
            CHECK(v1.size() == 3u);
            CHECK(it == v1.begin());
            CHECK(v1[0u] == boost::lexical_cast<T>(2));
            CHECK(v1[1u] == boost::lexical_cast<T>(3));
            CHECK(v1[2u] == boost::lexical_cast<T>(4));
            it = v1.erase(v1.begin() + 1);
            CHECK(v1.size() == 2u);
            CHECK(it == v1.begin() + 1);
            CHECK(v1[0u] == boost::lexical_cast<T>(2));
            CHECK(v1[1u] == boost::lexical_cast<T>(4));
            it = v1.erase(v1.begin());
            CHECK(v1.size() == 1u);
            CHECK(it == v1.begin());
            CHECK(v1[0u] == boost::lexical_cast<T>(4));
            it = v1.erase(v1.begin());
            CHECK(v1.size() == 0u);
            CHECK(it == v1.end());
        }
    }
};

TEST_CASE("small_vector_dynamic_test")
{
    boost::mpl::for_each<value_types>(dynamic_tester());
}

struct constructor_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1.size() == 0u);
            CHECK(v1.begin() == v1.end());
            CHECK(static_cast<v_type const &>(v1).begin() == static_cast<v_type const &>(v1).end());
            CHECK(v1.begin() == static_cast<v_type const &>(v1).end());
            CHECK(static_cast<v_type const &>(v1).begin() == v1.end());
            CHECK(v1.is_static());
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size) * 8 + 3),
                            [&n]() { return T(n++); });
            CHECK(!v1.is_static());
            v_type v2(v1);
            CHECK(!v2.is_static());
            CHECK(std::equal(v1.begin(), v1.end(), v2.begin()));
            v_type v3(std::move(v2));
            CHECK(std::equal(v1.begin(), v1.end(), v3.begin()));
            n = 0;
            v_type v4;
            std::generate_n(std::back_inserter(v4), safe_cast<int>(integer(v_type::max_static_size)),
                            [&n]() { return T(n++); });
            CHECK(v4.is_static());
            v_type v5(v4);
            CHECK(v5.is_static());
            CHECK(std::equal(v4.begin(), v4.end(), v5.begin()));
            v_type v6(std::move(v5));
            CHECK(std::equal(v4.begin(), v4.end(), v6.begin()));
            // Constructor from size and value.
            v_type v7(0, T(1));
            CHECK(v7.size() == 0u);
            v_type v8(1u, T(42));
            CHECK(v8.size() == 1u);
            CHECK((*v8.begin()) == T(42));
            v_type v9(3u, T(42));
            CHECK(v9.size() == 3u);
            CHECK((*v9.begin()) == T(42));
            CHECK(*(v9.begin() + 1) == T(42));
            CHECK(*(v9.begin() + 2) == T(42));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_constructor_test")
{
    boost::mpl::for_each<value_types>(constructor_tester());
}

struct assignment_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            v1.push_back(T(0));
            auto *ptr = std::addressof(v1[0]);
            // Verify that self assignment does not do anything funky.
            v1 = v1;
            v1 = std::move(v1);
            CHECK(ptr == std::addressof(v1[0]));
            v_type v2;
            // This will be static vs static (there is always enough static storage for at least 1 element).
            v2 = v1;
            CHECK(v2.size() == 1u);
            CHECK(v2[0] == v1[0]);
            // Push enough into v1 to make it dynamic.
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size)),
                            [&n]() { return T(n++); });
            CHECK(!v1.is_static());
            CHECK(v2.is_static());
            // Static vs dynamic.
            v2 = v1;
            CHECK(!v2.is_static());
            CHECK(std::equal(v2.begin(), v2.end(), v1.begin()));
            v_type v3;
            // Dynamic vs static.
            v1 = v3;
            CHECK(v1.is_static());
            CHECK(v1.size() == 0u);
            // Dynamic vs dynamic.
            v_type v4(v2), v5(v2);
            std::transform(v5.begin(), v5.end(), v5.begin(), [](const T &x) { return x / 2; });
            v4 = v5;
            CHECK(std::equal(v4.begin(), v4.end(), v5.begin()));
            v4 = std::move(v5);
            CHECK(v5.size() == 0u);
            CHECK(!v5.is_static());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_assignment_test")
{
    boost::mpl::for_each<value_types>(assignment_tester());
}

struct push_back_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            std::vector<T> check;
            CHECK(v1.size() == 0u);
            for (typename std::decay<decltype(v_type::max_static_size)>::type i = 0u; i < v_type::max_static_size;
                 ++i) {
                v1.push_back(T(i));
                check.push_back(T(i));
            }
            v1.push_back(T(5));
            check.push_back(T(5));
            v1.push_back(T(6));
            check.push_back(T(6));
            v1.push_back(T(7));
            check.push_back(T(7));
            CHECK(v1.size() == integer(v_type::max_static_size) + 3);
            CHECK(std::equal(check.begin(), check.end(), v1.begin()));
            check.resize(0u);
            v_type v2;
            CHECK(v2.size() == 0u);
            T tmp;
            for (typename std::decay<decltype(v_type::max_static_size)>::type i = 0u; i < v_type::max_static_size;
                 ++i) {
                tmp = T(i);
                check.push_back(tmp);
                v2.push_back(tmp);
            }
            tmp = T(5);
            v2.push_back(tmp);
            check.push_back(tmp);
            tmp = T(6);
            v2.push_back(tmp);
            check.push_back(tmp);
            tmp = T(7);
            v2.push_back(tmp);
            check.push_back(tmp);
            CHECK(v2.size() == integer(v_type::max_static_size) + 3);
            CHECK(std::equal(check.begin(), check.end(), v2.begin()));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_push_back_test")
{
    boost::mpl::for_each<value_types>(push_back_tester());
}

struct equality_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1 == v1);
            CHECK(!(v1 != v1));
            v_type v2 = v1;
            v1.push_back(T(0));
            CHECK(v2 != v1);
            CHECK(!(v2 == v1));
            CHECK(v1 != v2);
            CHECK(!(v1 == v2));
            v2.push_back(T(0));
            CHECK(v2 == v1);
            CHECK(!(v2 != v1));
            CHECK(v1 == v2);
            CHECK(!(v1 != v2));
            // Push enough into v1 to make it dynamic.
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size)),
                            [&n]() { return T(n++); });
            CHECK(v2 != v1);
            CHECK(!(v2 == v1));
            CHECK(v1 != v2);
            CHECK(!(v1 == v2));
            v2 = v1;
            CHECK(v2 == v1);
            CHECK(!(v2 != v1));
            CHECK(v1 == v2);
            CHECK(!(v1 != v2));
            v2.push_back(T(5));
            CHECK(v2 != v1);
            CHECK(!(v2 == v1));
            CHECK(v1 != v2);
            CHECK(!(v1 == v2));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_equality_test")
{
    boost::mpl::for_each<value_types>(equality_tester());
}

struct hash_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1.hash() == 0u);
            v1.push_back(T(2));
            CHECK(v1.hash() == std::hash<T>()(T(2)));
            // Push enough into v1 to make it dynamic.
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size)),
                            [&n]() { return T(n++); });
            std::hash<T> hasher;
            std::size_t retval = hasher(v1[0u]);
            for (decltype(v1.size()) i = 1u; i < v1.size(); ++i) {
                boost::hash_combine(retval, hasher(v1[i]));
            }
            CHECK(retval == v1.hash());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_hash_test")
{
    boost::mpl::for_each<value_types>(hash_tester());
}

struct resize_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            v1.resize(0);
            CHECK(v1.size() == 0u);
            v1.resize(1);
            CHECK(v1.size() == 1u);
            CHECK(v1[0] == T());
            auto ptr = &v1[0];
            v1.resize(v_type::max_static_size);
            std::vector<T> cmp;
            cmp.resize(v_type::max_static_size);
            CHECK(ptr == &v1[0]);
            CHECK(std::equal(v1.begin(), v1.end(), cmp.begin()));
            v1.resize(v_type::max_static_size + 1u);
            cmp.resize(v_type::max_static_size + 1u);
            CHECK(ptr != &v1[0]);
            CHECK(std::equal(v1.begin(), v1.end(), cmp.begin()));
            v1.resize(v_type::max_static_size + 2u);
            cmp.resize(v_type::max_static_size + 2u);
            ptr = &v1[0];
            CHECK(std::equal(v1.begin(), v1.end(), cmp.begin()));
            v1.resize(0);
            CHECK(v1.size() == 0u);
            v1.resize(1);
            CHECK(ptr == &v1[0]);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_resize_test")
{
    boost::mpl::for_each<value_types>(resize_tester());
}

// Class that throws after a few constructions.
struct time_bomb2 {
    time_bomb2() : m_vector(5) {}
    time_bomb2(time_bomb2 &&) = default;
    time_bomb2(const time_bomb2 &) = default;
    time_bomb2(int)
    {
        if (s_counter == 4u) {
            throw std::runtime_error("ka-pow!");
        }
        ++s_counter;
    }
    time_bomb2 &operator=(const time_bomb2 &) = default;
    time_bomb2 &operator=(time_bomb2 &&other) noexcept
    {
        m_vector = std::move(other.m_vector);
        return *this;
    }
    ~time_bomb2() noexcept {}
    std::vector<int> m_vector;
    static unsigned s_counter;
};

namespace piranha
{

template <>
struct safe_convert_impl<time_bomb2, int> {
    bool operator()(time_bomb2 &tb2, int n) const
    {
        try {
            tb2 = time_bomb2(n);
            return true;
        } catch (...) {
            return false;
        }
    }
};
}

unsigned time_bomb2::s_counter = 0u;

struct init_list_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1({1});
            CHECK(v1[0] == T(1));
            v_type v2({1, 2, 3});
            CHECK(v2[0] == T(1));
            CHECK(v2[1] == T(2));
            CHECK(v2[2] == T(3));
            v_type v3({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});
            std::vector<int> cmp({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});
            CHECK(v3.size() == cmp.size());
            CHECK(std::equal(v3.begin(), v3.end(), cmp.begin()));
            CHECK((std::is_constructible<v_type, std::initializer_list<int>>::value));
            CHECK((!std::is_constructible<v_type, std::initializer_list<time_bomb2>>::value));
            using v_type2 = small_vector<time_bomb2, U>;
            time_bomb2::s_counter = 0u;
            CHECK_THROWS_AS(v_type2({1, 2, 3, 4, 5, 6, 7}), std::invalid_argument);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_init_list_test")
{
    boost::mpl::for_each<value_types>(init_list_tester());
}

struct add_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1, v2, v3;
            v1.add(v3, v2);
            CHECK(v3.size() == 0u);
            v1.push_back(T(1));
            CHECK_THROWS_AS(v1.add(v3, v2), std::invalid_argument);
            CHECK(v1.size() == 1u);
            CHECK(*v1.begin() == T(1));
            v2.push_back(T(2));
            v1.add(v3, v2);
            CHECK(v3.size() == 1u);
            CHECK(v3[0] == T(3));
            v1 = v_type{1, 2, 3, 4, 5, 6};
            v2 = v_type{7, 8, 9, 0, 1, 2};
            v1.add(v3, v2);
            CHECK((v3 == v_type{8, 10, 12, 4, 6, 8}));
            v3.resize(0);
            v1.add(v3, v2);
            CHECK((v3 == v_type{8, 10, 12, 4, 6, 8}));
            v3.resize(100);
            v1.add(v3, v2);
            CHECK((v3 == v_type{8, 10, 12, 4, 6, 8}));
            v3.add(v3, v3);
            CHECK((v3 == v_type{8 * 2, 10 * 2, 12 * 2, 4 * 2, 6 * 2, 8 * 2}));
            v3.add(v3, v2);
            CHECK((v3 == v_type{8 * 2 + 7, 10 * 2 + 8, 12 * 2 + 9, 4 * 2, 6 * 2 + 1, 8 * 2 + 2}));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_add_test")
{
    boost::mpl::for_each<value_types>(add_tester());
}

struct sub_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1, v2, v3;
            v1.sub(v3, v2);
            CHECK(v3.size() == 0u);
            v1.push_back(T(1));
            CHECK_THROWS_AS(v1.sub(v3, v2), std::invalid_argument);
            CHECK(v1.size() == 1u);
            CHECK(*v1.begin() == T(1));
            v2.push_back(T(2));
            v1.sub(v3, v2);
            CHECK(v3.size() == 1u);
            CHECK(v3[0] == T(-1));
            v1 = v_type{1, 2, 3, 4, 5, 6};
            v2 = v_type{7, 8, 9, 0, 1, 2};
            v1.sub(v3, v2);
            CHECK((v3 == v_type{-6, -6, -6, 4, 4, 4}));
            v3.resize(0);
            v1.sub(v3, v2);
            CHECK((v3 == v_type{-6, -6, -6, 4, 4, 4}));
            v3.resize(100);
            v1.sub(v3, v2);
            CHECK((v3 == v_type{-6, -6, -6, 4, 4, 4}));
            v3.sub(v3, v3);
            CHECK((v3 == v_type{0, 0, 0, 0, 0, 0}));
            v3.sub(v3, v2);
            CHECK((v3 == v_type{-7, -8, -9, 0, -1, -2}));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_sub_test")
{
    boost::mpl::for_each<value_types>(sub_tester());
}

TEST_CASE("small_vector_print_sizes")
{
    std::cout << "Signed char: " << sizeof(small_vector<signed char>) << ','
              << detail::prepare_for_print(small_vector<signed char>::max_static_size) << ','
              << detail::prepare_for_print(small_vector<signed char>::max_dynamic_size) << ','
              << alignof(small_vector<signed char>) << '\n';
    std::cout << "Short      : " << sizeof(small_vector<short>) << ','
              << detail::prepare_for_print(small_vector<short>::max_static_size) << ','
              << detail::prepare_for_print(small_vector<short>::max_dynamic_size) << ',' << alignof(small_vector<short>)
              << '\n';
    std::cout << "Int        : " << sizeof(small_vector<int>) << ','
              << detail::prepare_for_print(small_vector<int>::max_static_size) << ','
              << detail::prepare_for_print(small_vector<int>::max_dynamic_size) << ',' << alignof(small_vector<int>)
              << '\n';
    std::cout << "Long       : " << sizeof(small_vector<long>) << ','
              << detail::prepare_for_print(small_vector<long>::max_static_size) << ','
              << detail::prepare_for_print(small_vector<long>::max_dynamic_size) << ',' << alignof(small_vector<long>)
              << '\n';
    std::cout << "Long long  : " << sizeof(small_vector<long long>) << ','
              << detail::prepare_for_print(small_vector<long long>::max_static_size) << ','
              << detail::prepare_for_print(small_vector<long long>::max_dynamic_size) << ','
              << alignof(small_vector<long long>) << '\n';
}

struct move_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            v1.push_back(T(1));
            v_type v2(std::move(v1));
            CHECK(v2.size() == 1u);
            CHECK(v2[0u] == T(1));
            CHECK(v1.size() == 0u);
            CHECK(v1.begin() == v1.end());
            CHECK(v1.is_static());
            CHECK(v2.is_static());
            v1 = std::move(v2);
            CHECK(v1.size() == 1u);
            CHECK(v1[0u] == T(1));
            CHECK(v2.size() == 0u);
            CHECK(v2.begin() == v2.end());
            CHECK(v2.is_static());
            CHECK(v1.is_static());
            v1 = v_type();
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size) + 1),
                            [&n]() { return T(n++); });
            CHECK(!v1.is_static());
            v_type v3(std::move(v1));
            CHECK(integer(v3.size()) == integer(v_type::max_static_size) + 1);
            CHECK(v1.size() == 0u);
            CHECK(v1.begin() == v1.end());
            CHECK(!v1.is_static());
            CHECK(!v3.is_static());
            v1 = std::move(v3);
            CHECK(integer(v1.size()) == integer(v_type::max_static_size) + 1);
            CHECK(v3.size() == 0u);
            CHECK(v3.begin() == v3.end());
            CHECK(!v3.is_static());
            CHECK(!v1.is_static());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_move_test")
{
    boost::mpl::for_each<value_types>(move_tester());
}

#if defined(PIRANHA_WITH_BOOST_S11N)

struct serialization_tester {
    template <typename T>
    void operator()(const T &)
    {
        using v_type = small_vector<int, T>;
        using size_type = typename v_type::size_type;
        std::uniform_int_distribution<int> int_dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        std::uniform_int_distribution<unsigned> size_dist(0u, 10u);
        v_type tmp;
        for (int i = 0; i < ntries; ++i) {
            // Create a vector of random size and with random contents.
            v_type v;
            const auto size = static_cast<size_type>(size_dist(rng));
            for (size_type j = 0u; j < size; ++j) {
                v.push_back(int_dist(rng));
            }
            // Serialize it.
            std::stringstream ss;
            {
                boost::archive::text_oarchive oa(ss);
                oa << v;
            }
            {
                boost::archive::text_iarchive ia(ss);
                ia >> tmp;
            }
            CHECK(tmp == v);
        }
        // Try with integer.
        using v_type2 = small_vector<integer, T>;
        v_type2 tmp2;
        for (int i = 0; i < ntries; ++i) {
            v_type2 v;
            const auto size = static_cast<size_type>(size_dist(rng));
            for (size_type j = 0u; j < size; ++j) {
                v.push_back(integer(int_dist(rng)));
            }
            // Serialize it.
            std::stringstream ss;
            {
                boost::archive::text_oarchive oa(ss);
                oa << v;
            }
            {
                boost::archive::text_iarchive ia(ss);
                ia >> tmp2;
            }
            CHECK(tmp2 == v);
        }
    }
};

TEST_CASE("small_vector_serialization_test")
{
    boost::mpl::for_each<size_types>(serialization_tester());
}

#endif

struct empty_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1.empty());
            CHECK(v1.is_static());
            v1.push_back(T(1));
            CHECK(!v1.empty());
            CHECK(v1.is_static());
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size) + 1),
                            [&n]() { return T(n++); });
            CHECK(!v1.is_static());
            CHECK(!v1.empty());
            v1.resize(0u);
            CHECK(!v1.is_static());
            CHECK(v1.empty());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_empty_test")
{
    boost::mpl::for_each<value_types>(empty_tester());
}

struct erase_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            if (U::value < 2u) {
                return;
            }
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1.empty());
            CHECK(v1.is_static());
            v1.push_back(T(1));
            auto it = v1.erase(v1.begin());
            CHECK(it == v1.end());
            CHECK(v1.empty());
            CHECK(v1.is_static());
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size) + 1),
                            [&n]() { return T(n++); });
            CHECK(!v1.is_static());
            CHECK(!v1.empty());
            it = v1.erase(v1.begin());
            CHECK(!v1.is_static());
            CHECK(!v1.empty());
            CHECK(it != v1.end());
            CHECK(*it == integer(1));
            CHECK(v1.size() == integer(v_type::max_static_size));
            it = v1.erase(v1.end() - 1);
            CHECK(!v1.is_static());
            CHECK(!v1.empty());
            CHECK(v1.size() == integer(v_type::max_static_size) - 1);
            CHECK(it == v1.end());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_erase_test")
{
    boost::mpl::for_each<value_types>(erase_tester());
}

struct size_be_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            if (U::value < 2u) {
                return;
            }
            using v_type = small_vector<T, U>;
            v_type v1;
            CHECK(v1.empty());
            CHECK(v1.is_static());
            auto t0 = v1.size_begin_end();
            CHECK(std::get<0>(t0) == 0u);
            CHECK(std::get<1>(t0) == std::get<2>(t0));
            CHECK(std::get<1>(t0) == v1.begin());
            CHECK(std::get<2>(t0) == v1.end());
            // Check the const counterpart too.
            auto t1 = static_cast<const v_type &>(v1).size_begin_end();
            CHECK(std::get<0>(t1) == 0u);
            CHECK(std::get<1>(t1) == std::get<2>(t1));
            CHECK(std::get<1>(t1) == static_cast<const v_type &>(v1).begin());
            CHECK(std::get<2>(t1) == static_cast<const v_type &>(v1).end());
            // Switch to dynamic.
            int n = 0;
            std::generate_n(std::back_inserter(v1), safe_cast<int>(integer(v_type::max_static_size) + 1),
                            [&n]() { return T(n++); });
            t0 = v1.size_begin_end();
            CHECK(std::get<0>(t0) == integer(v_type::max_static_size) + 1);
            CHECK(std::get<1>(t0) == v1.begin());
            CHECK(std::get<2>(t0) == v1.end());
            // Check again const.
            t1 = static_cast<const v_type &>(v1).size_begin_end();
            CHECK(std::get<0>(t1) == integer(v_type::max_static_size) + 1);
            CHECK(std::get<1>(t1) == static_cast<const v_type &>(v1).begin());
            CHECK(std::get<2>(t1) == static_cast<const v_type &>(v1).end());
            // Resize back to zero.
            v1.resize(0u);
            t0 = v1.size_begin_end();
            CHECK(std::get<0>(t0) == 0u);
            CHECK(std::get<1>(t0) == std::get<2>(t0));
            CHECK(std::get<1>(t0) == v1.begin());
            CHECK(std::get<2>(t0) == v1.end());
            t1 = static_cast<const v_type &>(v1).size_begin_end();
            CHECK(std::get<0>(t1) == 0u);
            CHECK(std::get<1>(t1) == std::get<2>(t1));
            CHECK(std::get<1>(t1) == static_cast<const v_type &>(v1).begin());
            CHECK(std::get<2>(t1) == static_cast<const v_type &>(v1).end());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("small_vector_size_begin_end_test")
{
    boost::mpl::for_each<value_types>(size_be_tester());
}
