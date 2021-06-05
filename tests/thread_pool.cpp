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

#include <piranha/thread_pool.hpp>

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <chrono>
#include <limits>
#include <list>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include <mp++/config.hpp>

#include <piranha/integer.hpp>
#if defined(MPPP_WITH_MPFR)
#include <piranha/real.hpp>
#endif
#include <piranha/runtime_info.hpp>
#include <piranha/type_traits.hpp>

#include "catch.hpp"
#include "exception_matcher.hpp"

using namespace piranha;

// Typedef for detection idiom.
template <typename F, typename... Args>
using enqueue_t = decltype(thread_pool::enqueue(0u, std::declval<F>(), std::declval<Args>()...));

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable &) = delete;
    noncopyable(noncopyable &&) = delete;
    ~noncopyable() = default;
    noncopyable &operator=(const noncopyable &) = delete;
    noncopyable &operator=(noncopyable &&) = delete;
};

struct noncopyable_functor {
    noncopyable_functor() = default;
    noncopyable_functor(const noncopyable_functor &) = delete;
    noncopyable_functor(noncopyable_functor &&) = delete;
    ~noncopyable_functor() = default;
    noncopyable_functor &operator=(const noncopyable_functor &) = delete;
    noncopyable_functor &operator=(noncopyable_functor &&) = delete;
    void operator()() const;
};

noncopyable noncopyable_ret_f();

void requires_move(int &&);

static int nn = 5;

struct ref_test_functor {
    template <typename T>
    void operator()(T &arg)
    {
        CHECK((std::is_same<T, int>::value));
        CHECK(&arg == &nn);
    }
};

struct cref_test_functor {
    template <typename T>
    void operator()(T &arg)
    {
        CHECK((std::is_same<T, const int>::value));
        CHECK(&arg == &nn);
    }
};

TEST_CASE("thread_pool_task_queue_test")
{
    std::cout << "thread_pool_task_queue_test" << std::endl <<std::flush;
    // A few simple tests.
    auto simple_00 = []() {};
    CHECK((is_detected<enqueue_t, decltype(simple_00)>::value));
    CHECK((is_detected<enqueue_t, decltype(simple_00) &>::value));
    CHECK((is_detected<enqueue_t, const decltype(simple_00) &>::value));
    CHECK((is_detected<enqueue_t, const decltype(simple_00)>::value));
    CHECK((!is_detected<enqueue_t, decltype(simple_00) &, int>::value));
    CHECK((!is_detected<enqueue_t, decltype(simple_00) &, void>::value));
    CHECK((!is_detected<enqueue_t, void, int>::value));
    CHECK((!is_detected<enqueue_t, decltype(simple_00) &, void>::value));

    // Check that noncopyable functor disables enqueue.
    CHECK((!is_detected<enqueue_t, noncopyable_functor &>::value));
    CHECK((!is_detected<enqueue_t, noncopyable_functor &&>::value));
    CHECK((!is_detected<enqueue_t, noncopyable_functor>::value));

    // Check that noncopyable return type disables enqueue.
    CHECK((!is_detected<enqueue_t, decltype(noncopyable_ret_f)>::value));

    // Check that if a function argument must be passed as rvalue ref, then enqueue is disabled.
    CHECK((!is_detected<enqueue_t, decltype(requires_move), int>::value));
    // Test that std::ref/cref works as intended (i.e., it does not copy the value) and that
    // reference_wrapper is removed when the argument is passed to the functor.
    auto fut_ref = thread_pool::enqueue(0, ref_test_functor{}, std::ref(nn));
    fut_ref.get();
    fut_ref = thread_pool::enqueue(0, cref_test_functor{}, std::cref(nn));
    fut_ref.get();
    auto slow_task = []() { std::this_thread::sleep_for(std::chrono::milliseconds(250)); };
    auto fast_task = [](int n) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return n;
    };
    auto instant_task = []() noexcept {};
    {
        task_queue tq(0);
    }
    {
        task_queue tq(0);
        tq.stop();
        tq.stop();
        tq.stop();
    }
    {
        task_queue tq(0);
        tq.enqueue([]() noexcept {});
        tq.stop();
        tq.stop();
    }
    {
        task_queue tq(0);
        tq.enqueue(slow_task);
        tq.stop();
        tq.stop();
    }
    {
        task_queue tq(0);
        tq.enqueue(slow_task);
        tq.enqueue(slow_task);
        tq.enqueue(slow_task);
    }
    {
        task_queue tq(0);
        auto f1 = tq.enqueue(slow_task);
        auto f2 = tq.enqueue(slow_task);
        auto f3 = tq.enqueue(slow_task);
        f3.get();
    }
    {
        task_queue tq(0);
        auto f1 = tq.enqueue([](int) { throw std::runtime_error(""); }, 1);
        CHECK_THROWS_AS(f1.get(), std::runtime_error);
    }
    {
        task_queue tq(0);
        auto f1 = tq.enqueue([](int n) noexcept { return n + n; }, 45);
        CHECK(f1.get() == 90);
    }
    {
        task_queue tq(0);
        using f_type = decltype(tq.enqueue(fast_task, 0));
        std::list<f_type> l;
        for (int i = 0; i < 100; ++i) {
            l.push_back(tq.enqueue(fast_task, i));
        }
        tq.stop();
        int result = 0;
        for (f_type &f : l) {
            result += f.get();
        }
        CHECK(result == 4950);
    }
    {
        task_queue tq(0);
        for (int i = 0; i < 10000; ++i) {
            tq.enqueue(instant_task);
        }
        tq.stop();
        CHECK_THROWS_MATCHES(tq.enqueue(instant_task), std::runtime_error,
            test::ExceptionMatcher<std::runtime_error>(std::string("cannot enqueue task while the task queue is stopping"))
        );
    }
    {
        task_queue tq(0);
        noncopyable nc;
        tq.enqueue([](noncopyable &) noexcept {}, std::ref(nc));
        tq.enqueue([](const noncopyable &) noexcept {}, std::cref(nc));
    }
#if defined(MPPP_WITH_MPFR)
    {
        task_queue tq(0);
        for (int i = 0; i < 100; ++i) {
            tq.enqueue([]() { mppp::real_pi(500); });
        }
    }
#endif
}

static int adder(int a, int b)
{
    return a + b;
}

TEST_CASE("thread_pool_test")
{
    std::cout << "thread_pool_test" << std::endl << std::flush;
    const unsigned initial_size = thread_pool::size();
    CHECK(initial_size > 0u);
    CHECK(thread_pool::enqueue(0, adder, 1, 2).get() == 3);
    thread_pool::enqueue(0, []() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); });
    CHECK(thread_pool::enqueue(0, adder, 4, -5).get() == -1);
    CHECK_THROWS_MATCHES(
        thread_pool::enqueue(initial_size, adder, 4, -5), std::invalid_argument,
        test::ExceptionMatcher<std::invalid_argument>(std::string("the thread pool contains only "))
    );
    CHECK_THROWS_AS(thread_pool::enqueue(0, []() { throw std::runtime_error(""); }).get(), std::runtime_error);
    auto fast_task = [](int n) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return n;
    };
    for (unsigned i = 0u; i < initial_size; ++i) {
        for (int n = 0; n < 1000; ++n) {
            thread_pool::enqueue(i, fast_task, n);
        }
    }
    for (unsigned i = 0u; i < initial_size; ++i) {
        thread_pool::enqueue(i, []() noexcept {}).get();
    }
    auto slow_task = []() { std::this_thread::sleep_for(std::chrono::milliseconds(250)); };
    thread_pool::resize(1);
    thread_pool::enqueue(0, slow_task);
    thread_pool::resize(20u);
    CHECK(thread_pool::size() == 20u);
    thread_pool::resize(1);
    thread_pool::enqueue(0, slow_task);
    thread_pool::resize(20u);
    CHECK(thread_pool::size() == 20u);
    for (unsigned i = 0u; i < 20u; ++i) {
        thread_pool::enqueue(0u, slow_task);
        for (int n = 1; n < 1000; ++n) {
            thread_pool::enqueue(i, fast_task, n);
        }
    }
    CHECK(thread_pool::size() == 20u);
    thread_pool::resize(10u);
    CHECK(thread_pool::size() == 10u);
    CHECK_THROWS_MATCHES(thread_pool::resize(0u), std::invalid_argument, 
         test::ExceptionMatcher<std::invalid_argument>(std::string("cannot resize the thread pool to zero"))
    );
    CHECK(thread_pool::size() != 0u);
}

TEST_CASE("thread_pool_future_list_test")
{
    std::cout << "thread_pool_future_list_test"  << std::endl << std::flush;
    thread_pool::resize(10u);
    auto null_task = []() {};
    future_list<decltype(null_task())> f1;
    CHECK(!std::is_copy_assignable<decltype(f1)>::value);
    CHECK(!std::is_move_assignable<decltype(f1)>::value);
    f1.wait_all();
    f1.wait_all();
    f1.get_all();
    f1.get_all();
    auto fast_task = []() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); };
    future_list<decltype(fast_task())> f2;
    for (unsigned i = 0u; i < 10u; ++i) {
        for (unsigned j = 0u; j < 100u; ++j) {
            f2.push_back(thread_pool::enqueue(i, fast_task));
        }
    }
    f2.wait_all();
    f2.wait_all();
    f2.get_all();
    f2.get_all();
    auto thrower = []() { throw std::runtime_error(""); };
    future_list<decltype(thrower())> f3;
    for (unsigned i = 0u; i < 10u; ++i) {
        for (unsigned j = 0u; j < 100u; ++j) {
            f3.push_back(thread_pool::enqueue(i, thrower));
        }
    }
    f3.wait_all();
    f3.wait_all();
    CHECK_THROWS_AS(f3.get_all(), std::runtime_error);
    CHECK_THROWS_AS(f3.get_all(), std::runtime_error);
    CHECK_THROWS_AS(f3.get_all(), std::runtime_error);
    // Try with empty futures.
    future_list<decltype(thrower())> f4;
    for (unsigned i = 0u; i < 100u; ++i) {
        f4.push_back(decltype(thread_pool::enqueue(0, thrower))());
    }
    f4.wait_all();
    f4.wait_all();
    f4.get_all();
    f4.get_all();
}

TEST_CASE("thread_pool_use_threads_test")
{
    std::cout << "thread_pool_use_threads_test" << std::endl << std::flush;
    thread_pool::resize(4u);
    CHECK(thread_pool::use_threads(100u, 3u) == 4u);
    CHECK_THROWS_MATCHES(
        thread_pool::use_threads(100u, 0u), std::invalid_argument, 
         test::ExceptionMatcher<std::invalid_argument>(std::string("invalid value of 0 for minimum work per thread (it must be strictly positive)"))
    );
    CHECK_THROWS_MATCHES(
        thread_pool::use_threads(0u, 100u), std::invalid_argument, 
         test::ExceptionMatcher<std::invalid_argument>(std::string("invalid value of 0 for work size (it must be strictly positive)"))
    );
    CHECK_THROWS_AS(thread_pool::use_threads(0u, 0u), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(100_z, 0_z), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(0_z, 100_z), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(0_z, 0_z), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(100_z, -1_z), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(-1_z, 100_z), std::invalid_argument);
    CHECK_THROWS_AS(thread_pool::use_threads(-1_z, -1_z), std::invalid_argument);
    CHECK(thread_pool::use_threads(100u, 30u) == 3u);
    auto f1 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 3u); });
    auto f2 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 1u); });
    auto f3 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 0u); });
    CHECK(f1.get() == 1u);
    CHECK(f2.get() == 1u);
    CHECK_THROWS_AS(f3.get(), std::invalid_argument);
    thread_pool::resize(1u);
    CHECK(thread_pool::use_threads(100u, 3u) == 1u);
    CHECK_THROWS_AS(thread_pool::use_threads(100u, 0u), std::invalid_argument);
    CHECK(thread_pool::use_threads(100u, 30u) == 1u);
    auto f4 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 3u); });
    auto f5 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 1u); });
    auto f6 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(100u, 0u); });
    CHECK(f4.get() == 1u);
    CHECK(f5.get() == 1u);
    CHECK_THROWS_AS(f6.get(), std::invalid_argument);
    thread_pool::resize(4u);
    CHECK(thread_pool::use_threads(integer(100u), integer(3u)) == 4u);
    CHECK_THROWS_AS(thread_pool::use_threads(integer(100u), integer(0u)), std::invalid_argument);
    CHECK(thread_pool::use_threads(integer(100u), integer(30u)) == 3u);
    auto f7 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(integer(100u), integer(3u)); });
    auto f8 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(integer(100u), integer(1u)); });
    auto f9 = thread_pool::enqueue(0u, []() { return thread_pool::use_threads(integer(100u), integer(0u)); });
    CHECK(f7.get() == 1u);
    CHECK(f8.get() == 1u);
    CHECK_THROWS_AS(f9.get(), std::invalid_argument);
}
