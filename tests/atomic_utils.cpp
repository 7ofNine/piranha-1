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

#include <piranha/detail/atomic_flag_array.hpp>
#include <piranha/detail/atomic_lock_guard.hpp>


#include <algorithm>
#include <cstddef>
#include <thread>
#include <type_traits>
#include <vector>
#include <barrier>

//#include <piranha/thread_barrier.hpp>

#include "catch.hpp"

using namespace piranha;

using a_array = detail::atomic_flag_array;
using alg = detail::atomic_lock_guard;

TEST_CASE("atomic_utils_atomic_flag_array_test")
{
    // Test with just an empty array.
    a_array a0(0u);
    // Non-empty.
    std::size_t size = 100u;
    a_array a1(size);
    // Verify everything is set to false.
    for (std::size_t i = 0u; i < size; ++i) {
        CHECK(!a1[i].test_and_set());
        CHECK(a1[i].test_and_set());
    }
    // Concurrent.
    size = 1000000u;
    a_array a2(size);
    std::barrier tb(2u, []() noexcept {});
    auto func = [&a2, &tb, size]() {
        tb.arrive_and_wait();
        for (std::size_t i = 0u; i < size; ++i) {
            a2[i].test_and_set();
        }
    };
    std::thread t0(func);
    std::thread t1(func);
    t0.join();
    t1.join();
    for (std::size_t i = 0u; i < size; ++i) {
        CHECK(a2[i].test_and_set());
        // Check also the const getter of the array.
        CHECK(std::addressof(a2[i]) == std::addressof(static_cast<const a_array &>(a2)[i]));
    }
    // Some type traits checks.
    CHECK(!std::is_constructible<a_array>::value);
    CHECK(!std::is_copy_constructible<a_array>::value);
    CHECK(!std::is_move_constructible<a_array>::value);
    CHECK(!std::is_copy_assignable<a_array>::value);
    CHECK(!std::is_move_assignable<a_array>::value);
}

TEST_CASE("atomic_utils_atomic_lock_guard_test")
{
    // Some type traits checks.
    CHECK(!std::is_constructible<alg>::value);
    CHECK(!std::is_copy_constructible<alg>::value);
    CHECK(!std::is_move_constructible<alg>::value);
    CHECK(!std::is_copy_assignable<alg>::value);
    CHECK(!std::is_move_assignable<alg>::value);
    // Concurrent writes protected by a spinlock.
    std::size_t size = 10000u;
    using size_type = std::vector<double>::size_type;
    std::vector<double> v(size, 0.);
    a_array a0(size);
    std::barrier tb(2u, []()noexcept {});
    auto func = [&a0, &tb, &v, size]() {
        tb.arrive_and_wait();
        for (std::size_t i = 0u; i < size; ++i) {
            alg l(a0[i]);
            v[static_cast<size_type>(i)] = 1.;
        }
    };
    std::thread t0(func);
    std::thread t1(func);
    t0.join();
    t1.join();
    CHECK(std::all_of(v.begin(), v.end(), [](double x) { return x == 1.; }));
}
