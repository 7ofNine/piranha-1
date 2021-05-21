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

#include <piranha/static_vector.hpp>

#include <boost/integer_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <piranha/integer.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"

// NOTE: here we define a custom string class base on std::string that respects nothrow requirements in hash_set:
// in the current GCC (4.6) the destructor of std::string does not have nothrow, so we cannot use it.
class custom_string : public std::string
{
public:
    custom_string() = default;
    custom_string(const custom_string &) = default;
    // NOTE: strange thing here, move constructor of std::string results in undefined reference?
    custom_string(custom_string &&other) noexcept : std::string(other) {}
    template <typename... Args>
    custom_string(Args &&... params) : std::string(std::forward<Args>(params)...)
    {
    }
    custom_string &operator=(const custom_string &) = default;
    custom_string &operator=(custom_string &&other) noexcept
    {
        std::string::operator=(std::move(other));
        return *this;
    }
    ~custom_string() {}
};

namespace std
{

template <>
struct hash<custom_string> {
    size_t operator()(const custom_string &s) const
    {
        return hash<std::string>()(s);
    }
};
}

using namespace piranha;

static_assert(is_hashable<custom_string>::value, "cc");

typedef boost::mpl::vector<int, /*integer,*/ custom_string> value_types;  // integer doesn't have an input operator>>
typedef boost::mpl::vector<std::integral_constant<std::uint_least8_t, 1u>,
                           std::integral_constant<std::uint_least8_t, 5u>,
                           std::integral_constant<std::uint_least8_t, 10u>>
    size_types;

// Constructors, assignments and element access.
struct constructor_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            {
                // Default constructor.
                vector_type v;
                CHECK(v.size() == 0u);
                CHECK(vector_type(v).size() == 0u);
                CHECK(vector_type(std::move(v)).size() == 0u);
                v = vector_type();
                v.push_back(boost::lexical_cast<T>(1));
                CHECK(v.size() == 1u);
                CHECK(v[0] == boost::lexical_cast<T>(1));
                // Copy constructor.
                CHECK(vector_type(v).size() == 1u);
                CHECK(vector_type(v)[0] == boost::lexical_cast<T>(1));
                // Move constructor.
                CHECK(vector_type(std::move(v))[0] == boost::lexical_cast<T>(1));
                // Copy assignment.
                vector_type tmp;
                tmp.push_back(boost::lexical_cast<T>(1));
                v = tmp;
                CHECK(v.size() == 1u);
                CHECK(v[0] == boost::lexical_cast<T>(1));
                // Move assignment.
                v = vector_type();
                v.push_back(boost::lexical_cast<T>(1));
                CHECK(v.size() == 1u);
                CHECK(v[0] == boost::lexical_cast<T>(1));
                // Mutating accessor.
                v[0] = boost::lexical_cast<T>(2);
                CHECK(v[0] == boost::lexical_cast<T>(2));
            }
            if (U::value > 1u) {
                // Move Assignment with different sizes.
                vector_type v, u;
                v.push_back(boost::lexical_cast<T>(1));
                v.push_back(boost::lexical_cast<T>(2));
                u.push_back(boost::lexical_cast<T>(3));
                v = std::move(u);
                CHECK(v.size() == 1u);
                CHECK(v[0] == boost::lexical_cast<T>(3));
                u = vector_type();
                v = vector_type();
                v.push_back(boost::lexical_cast<T>(1));
                v.push_back(boost::lexical_cast<T>(2));
                u.push_back(boost::lexical_cast<T>(3));
                u = std::move(v);
                CHECK(u.size() == 2u);
                CHECK(u[0] == boost::lexical_cast<T>(1));
                CHECK(u[1] == boost::lexical_cast<T>(2));
            }
            // Constructor from copies.
            vector_type v(0u, boost::lexical_cast<T>(1));
            CHECK(v.size() == 0u);
            v = vector_type(1u, boost::lexical_cast<T>(2));
            CHECK(v.size() == 1u);
            CHECK(v[0u] == boost::lexical_cast<T>(2));
            CHECK_THROWS_AS(v = vector_type(1u + U::value, boost::lexical_cast<T>(2)), std::bad_alloc);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_constructor_test")
{
    boost::mpl::for_each<value_types>(constructor_tester());
}

struct iterator_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v;
            CHECK(v.begin() == v.end());
            v.push_back(boost::lexical_cast<T>(1));
            auto it = v.begin();
            ++it;
            CHECK(it == v.end());
            CHECK(std::distance(v.begin(), v.end()) == 1);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_iterator_test")
{
    boost::mpl::for_each<value_types>(iterator_tester());
}

TEST_CASE("static_vector_size_type_test")
{
    CHECK((std::is_same<detail::static_vector_size_type<10u>::type, unsigned char>::value));
    CHECK((std::is_same<detail::static_vector_size_type<255u>::type, unsigned char>::value));
    CHECK(((std::is_same<detail::static_vector_size_type<10000u>::type, unsigned char>::value)
                || (std::is_same<detail::static_vector_size_type<10000u>::type, unsigned short>::value)));
    CHECK(((std::is_same<detail::static_vector_size_type<4294967295ul>::type, unsigned char>::value)
                || (std::is_same<detail::static_vector_size_type<4294967295ul>::type, unsigned short>::value)
                || (std::is_same<detail::static_vector_size_type<4294967295ul>::type, unsigned>::value)
                || (std::is_same<detail::static_vector_size_type<4294967295ul>::type, unsigned long>::value)));
}

struct equality_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            CHECK(vector_type() == vector_type());
            vector_type v1, v2;
            v1.push_back(boost::lexical_cast<T>(1));
            CHECK(!(v1 == v2));
            CHECK(v1 != v2);
            v2.push_back(boost::lexical_cast<T>(1));
            CHECK(v1 == v2);
            CHECK(!(v1 != v2));
            v1 = vector_type();
            v1.push_back(boost::lexical_cast<T>(2));
            CHECK(!(v1 == v2));
            CHECK(v1 != v2);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_equality_test")
{
    boost::mpl::for_each<value_types>(equality_tester());
}

struct push_back_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v;
            // Move-pushback.
            v.push_back(boost::lexical_cast<T>(1));
            CHECK(v.size() == 1u);
            CHECK(v[0] == boost::lexical_cast<T>(1));
            // Copy-pushback.
            auto tmp = boost::lexical_cast<T>(1);
            v = vector_type();
            v.push_back(tmp);
            CHECK(v.size() == 1u);
            CHECK(v[0] == boost::lexical_cast<T>(1));
            // Check for throw.
            for (auto i = v.size(); i < U::value; ++i) {
                v.push_back(tmp);
            }
            CHECK_THROWS_AS(v.push_back(tmp), std::bad_alloc);
            CHECK_THROWS_AS(v.push_back(std::move(tmp)), std::bad_alloc);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_push_back_test")
{
    boost::mpl::for_each<value_types>(push_back_tester());
}

struct emplace_back_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v;
            v.emplace_back(boost::lexical_cast<T>(1));
            CHECK(v.size() == 1u);
            CHECK(v[0] == boost::lexical_cast<T>(1));
            // Check for throw.
            for (auto i = v.size(); i < U::value; ++i) {
                v.emplace_back(boost::lexical_cast<T>(1));
            }
            CHECK_THROWS_AS(v.emplace_back(boost::lexical_cast<T>(1)), std::bad_alloc);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_emplace_back_test")
{
    boost::mpl::for_each<value_types>(emplace_back_tester());
}

// Class that throws after a few default constructions.
struct time_bomb {
    time_bomb() : m_vector(5)
    {
        if (s_counter == 2u) {
            throw std::runtime_error("ka-pow!");
        }
        ++s_counter;
    }
    time_bomb(time_bomb &&) = default;
    time_bomb(const time_bomb &) = default;
    time_bomb &operator=(const time_bomb &) = default;
    time_bomb &operator=(time_bomb &&other) noexcept
    {
        m_vector = std::move(other.m_vector);
        return *this;
    }
    bool operator==(const time_bomb &other) const
    {
        return m_vector == other.m_vector;
    }
    ~time_bomb() {}
    std::vector<int> m_vector;
    static unsigned s_counter;
};

unsigned time_bomb::s_counter = 0u;

struct resize_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v;
            v.resize(1u);
            CHECK_THROWS_AS(v.resize(U::value + 1u), std::bad_alloc);
            CHECK(v.size() == 1u);
            CHECK(v[0] == T());
            v.resize(1u);
            CHECK(v.size() == 1u);
            CHECK(v[0] == T());
            v.resize(0u);
            CHECK(v.size() == 0u);
            if (U::value < 3u) {
                return;
            }
            typedef static_vector<time_bomb, U::value> vector_type2;
            time_bomb::s_counter = 0u;
            vector_type2 v2;
            v2.resize(1);
            v2.resize(2);
            CHECK_THROWS_AS(v2.resize(3), std::runtime_error);
            CHECK(v2.size() == 2u);
            time_bomb::s_counter = 0u;
            CHECK(v2[0] == time_bomb());
            CHECK(v2[1] == time_bomb());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_resize_test")
{
    boost::mpl::for_each<value_types>(resize_tester());
}

struct stream_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v;
            std::ostringstream oss1;
            oss1 << v;
            CHECK(!oss1.str().empty());
            v.push_back(boost::lexical_cast<T>(1));
            if (U::value > 1u) {
                v.push_back(boost::lexical_cast<T>(1));
            }
            std::ostringstream oss2;
            oss2 << v;
            CHECK(!oss2.str().empty());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_stream_test")
{
    boost::mpl::for_each<value_types>(stream_tester());
}

struct type_traits_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            CHECK(is_container_element<vector_type>::value);
            CHECK(is_ostreamable<vector_type>::value);
            CHECK(is_equality_comparable<const vector_type &>::value);
            CHECK(!is_addable<vector_type>::value);
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_type_traits_test")
{
    boost::mpl::for_each<value_types>(type_traits_tester());
}

struct hash_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v1;
            CHECK(v1.hash() == 0u);
            v1.push_back(T());
            CHECK(v1.hash() == std::hash<T>()(T()));
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_hash_test")
{
    boost::mpl::for_each<value_types>(hash_tester());
}

// Move semantics tester.
struct move_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v1;
            v1.push_back(T());
            vector_type v2(std::move(v1));
            CHECK(T() == v2[0u]);
            CHECK(v1.size() == 0u);
            CHECK(v1.begin() == v1.end());
            v1 = std::move(v2);
            CHECK(T() == v1[0u]);
            CHECK(v2.size() == 0u);
            CHECK(v2.begin() == v2.end());
            if (U::value > 1u) {
                v1.push_back(boost::lexical_cast<T>("2"));
                v1.push_back(boost::lexical_cast<T>("3"));
                vector_type v3(std::move(v1));
                CHECK(v3.size() == 3u);
                CHECK(v3[0u] == T());
                CHECK(v3[1u] == boost::lexical_cast<T>("2"));
                CHECK(v3[2u] == boost::lexical_cast<T>("3"));
                CHECK(v1.size() == 0u);
                CHECK(v1.begin() == v1.end());
                v1 = std::move(v3);
                CHECK(v1.size() == 3u);
                CHECK(v1[0u] == T());
                CHECK(v1[1u] == boost::lexical_cast<T>("2"));
                CHECK(v1[2u] == boost::lexical_cast<T>("3"));
                CHECK(v3.size() == 0u);
                CHECK(v3.begin() == v3.end());
            }
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_move_semantics_test")
{
    boost::mpl::for_each<value_types>(move_tester());
}

struct empty_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            typedef static_vector<T, U::value> vector_type;
            vector_type v1;
            CHECK(v1.empty());
            v1.push_back(T());
            CHECK(!v1.empty());
            v1.resize(0u);
            CHECK(v1.empty());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_empty_test")
{
    boost::mpl::for_each<value_types>(empty_tester());
}

struct erase_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            if (U::value == 1u) {
                return;
            }
            typedef static_vector<T, U::value> vector_type;
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
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_erase_test")
{
    boost::mpl::for_each<value_types>(erase_tester());
}

struct clear_tester {
    template <typename T>
    struct runner {
        template <typename U>
        void operator()(const U &)
        {
            if (U::value == 1u) {
                return;
            }
            typedef static_vector<T, U::value> vector_type;
            vector_type v1;
            v1.push_back(boost::lexical_cast<T>(1));
            v1.clear();
            CHECK(v1.empty());
            v1.push_back(boost::lexical_cast<T>(1));
            v1.push_back(boost::lexical_cast<T>(2));
            CHECK(v1.size() == 2u);
            v1.clear();
            CHECK(v1.empty());
        }
    };
    template <typename T>
    void operator()(const T &)
    {
        boost::mpl::for_each<size_types>(runner<T>());
    }
};

TEST_CASE("static_vector_clear_test")
{
    boost::mpl::for_each<value_types>(clear_tester());
}
