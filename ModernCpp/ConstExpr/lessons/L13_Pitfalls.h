// =============================================================================
// L13 [C++20]  pitfalls, limits and debugging tips
// =============================================================================
// 1. constexpr on a function does NOT mean "computed at compile time"
// 2. Constant evaluation is a free undefined-behaviour detector - use it
// 3. Compilers limit recursion depth and evaluation steps
// 4. Floating point: compare with a tolerance
// 5. constexpr pointers: the POINTER is const, and only static addresses count
// 6. Debugging constexpr code
// (Tagged C++20 because it uses consteval and std::midpoint; the pitfalls
//  themselves apply from C++11 on.)
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <climits>
#include <numeric>

namespace l13_pitfalls {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. "constexpr" is permission, not a command
// -----------------------------------------------------------------------------
constexpr unsigned long long factorial(unsigned n)
{
    unsigned long long r = 1;
    for (unsigned i = 2; i <= n; ++i)
        r *= i;
    return r;
}

// unsigned long long x = factorial(20);
//   The compiler MAY compute this at compile time (an optimised Release build
//   usually does), but it doesn't have to: in a Debug build it is normally a
//   real function call at runtime. Ways to FORCE compile-time evaluation:
constexpr unsigned long long kForced1 = factorial(20);   // a) constexpr variable
static_assert(factorial(20) == 2432902008176640000ULL);  // b) static_assert

template <auto V>
inline constexpr auto force_constant = V;                // c) template argument

consteval auto as_constant(auto v) { return v; }        // d) consteval wrapper (C++20)

// -----------------------------------------------------------------------------
// 2. Use static_assert as a unit test - it catches undefined behaviour
// -----------------------------------------------------------------------------
// At runtime, signed overflow silently gives garbage. During constant
// evaluation it is a compile error. So test your constexpr functions with
// static_assert, including the edge cases.
constexpr int naive_midpoint(int a, int b) { return (a + b) / 2; }   // overflows for big values

static_assert(naive_midpoint(2, 8) == 5);
static_assert(naive_midpoint(-4, 4) == 0);

// ERROR DEMO [L13-E1]  fails in: every standard
// The edge-case test finds the bug: INT_MAX - 2 + INT_MAX overflows.
#if TUTORIAL_FORCE_ERRORS
static_assert(naive_midpoint(INT_MAX - 2, INT_MAX) == INT_MAX - 1);
#endif

// The fix: std::midpoint (C++20, constexpr) never overflows.
static_assert(std::midpoint(INT_MAX - 2, INT_MAX) == INT_MAX - 1);

// -----------------------------------------------------------------------------
// 3. Limits on constant evaluation
// -----------------------------------------------------------------------------
// Every compiler stops runaway compile-time code:
//                    recursion depth (default)     evaluation steps
//   MSVC             /constexpr:depth  (512)       /constexpr:steps  (1048576)
//   GCC              -fconstexpr-depth (512)       -fconstexpr-ops-limit (2^25)
//   Clang            -fconstexpr-depth (512)       -fconstexpr-steps (1048576)
// If you really need more, raise the limit - but first try a loop instead of
// recursion, or a smaller table.
constexpr int count_down(int n) { return n == 0 ? 0 : 1 + count_down(n - 1); }
static_assert(count_down(300) == 300);   // fine: below the depth limit

// ERROR DEMO [L13-E2]  fails in: every standard
// 10000 nested calls exceeds the default recursion depth.
#if TUTORIAL_FORCE_ERRORS
static_assert(count_down(10000) == 10000);
#endif

// ERROR DEMO [L13-E3]  fails in: every standard
// A loop with ~1 billion iterations exceeds the step limit. (The compiler
// gives up after the limit, so this costs a few seconds, not hours.)
#if TUTORIAL_FORCE_ERRORS
constexpr long long sum_below(long long n)
{
    long long s = 0;
    for (long long i = 0; i < n; ++i)
        s += i;
    return s;
}
static_assert(sum_below(1000000000LL) > 0);
#endif

// -----------------------------------------------------------------------------
// 4. Floating point
// -----------------------------------------------------------------------------
// Compile-time floating point follows the same IEEE rules, so rounding errors
// happen at compile time too. Never compare computed doubles with ==.
constexpr double sum_tenths()
{
    double s = 0.0;
    for (int i = 0; i < 10; ++i)
        s += 0.1;
    return s;
}
static_assert(sum_tenths() != 1.0);   // 0.9999999999999999

constexpr bool nearly_equal(double a, double b, double eps = 1e-12)
{
    return (a > b ? a - b : b - a) <= eps;
}
static_assert(nearly_equal(sum_tenths(), 1.0));

// -----------------------------------------------------------------------------
// 5. constexpr pointers
// -----------------------------------------------------------------------------
// `constexpr const char* p` means: p itself is a constant (can't be re-pointed)
// AND it points to const char. `constexpr` always applies to the variable.
constexpr const char* kNoteName = "A4";

// ERROR DEMO [L13-E4]  fails in: every standard
// kNoteName is a constant pointer: it can't be changed to point elsewhere.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_repoint() { kNoteName = "B4"; }
#endif

// Only addresses of objects with STATIC storage duration are constants.
inline int g_sampleRate = 48000;
constexpr int* kSampleRatePtr = &g_sampleRate;   // OK: the address is fixed

// ERROR DEMO [L13-E5]  fails in: every standard
// A local variable lives on the stack: its address differs on every call.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_local_address()
{
    int local = 1;
    constexpr int* p = &local;
    (void)p;
}
#endif

// -----------------------------------------------------------------------------
// 6. Debugging constexpr code
// -----------------------------------------------------------------------------
//  * You can't step through compile-time evaluation in a debugger. Call the
//    function with a runtime argument (e.g. a volatile variable) and step
//    through THAT - it is the same code.
//  * Break a failing static_assert into smaller ones to find the bad step:
//        static_assert(step1(x) == ...);  static_assert(step2(step1(x)) == ...);
//  * MSVC and Clang print the call stack of a failed constant evaluation
//    ("note: in call to ..."). Read the notes, not just the first error.
//  * Not sure whether something ran at compile time? Wrap it in as_constant()
//    or assign it to a constexpr variable: if it can't be constant, the build
//    fails and tells you why.

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L13", "pitfalls, limits, debugging");

    tut::section(STD, "1. forcing compile-time evaluation");
    tut::show(STD, "a) constexpr variable", kForced1);
    tut::show(STD, "c) force_constant<factorial(15)>", force_constant<factorial(15)>);
    tut::show(STD, "d) as_constant(factorial(10))", as_constant(factorial(10)));
    volatile unsigned n = 12;
    tut::show(STD, "factorial(n), runtime n = 12", factorial(n));

    tut::section(STD, "2. static_assert as a unit test");
    tut::show(STD, "std::midpoint(INT_MAX - 2, INT_MAX)", std::midpoint(INT_MAX - 2, INT_MAX));

    tut::section(STD, "3. limits");
    tut::show(STD, "count_down(300)", count_down(300));

    tut::section(STD, "4. floating point");
    tut::show(STD, "sum_tenths() == 1.0", sum_tenths() == 1.0);
    tut::show(STD, "nearly_equal(sum_tenths(), 1.0)", nearly_equal(sum_tenths(), 1.0));

    tut::section(STD, "5. constexpr pointers");
    tut::show(STD, "kNoteName", kNoteName);
    tut::show(STD, "*kSampleRatePtr", *kSampleRatePtr);
}

} // namespace l13_pitfalls

#endif // TUTORIAL_INCLUDE(20)
