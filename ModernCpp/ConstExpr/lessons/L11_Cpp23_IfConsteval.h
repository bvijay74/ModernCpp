// =============================================================================
// L11 [C++23]  if consteval, and consteval propagation
// =============================================================================
//     if consteval { A } else { B }      A: during constant evaluation
//                                        B: at runtime
//     if !consteval { B } else { A }
//
// Why it replaces std::is_constant_evaluated() (L08):
//   * inside `if consteval { ... }` you may call consteval functions with the
//     function's own parameters - the compiler knows that branch only ever
//     runs during constant evaluation (L08-E1 showed this failing before)
//   * it can't be misused as `if constexpr (std::is_constant_evaluated())`
//   * no header needed; the braces are mandatory
//
// Consteval propagation (P2564, a C++23 change): a function TEMPLATE or a
// LAMBDA that calls a consteval function with its own parameter silently
// becomes consteval itself, instead of being an error (L06-E2).
//
// Compiler support is checked with feature-test macros:
//   if consteval            __cpp_if_consteval >= 202106L
//   consteval propagation   __cpp_consteval    >= 202211L  (GCC 14, Clang 17)
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(23)

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace l11_cpp23_if_consteval {

const int STD = 23;

#if defined(__cpp_if_consteval) && __cpp_if_consteval >= 202106L
#define L11_HAS_IF_CONSTEVAL 1

// -----------------------------------------------------------------------------
// 1. if consteval: two implementations, one function
// -----------------------------------------------------------------------------
constexpr const char* where()
{
    if consteval {
        return "compile time";
    }
    else {
        return "runtime";
    }
}
constexpr const char* kWhere = where();

// Same idea as L08's portable_sqrt, written with if consteval.
constexpr double portable_sqrt(double x)
{
    if consteval {
        double guess = x > 0 ? x : 0;
        for (int i = 0; i < 64 && guess > 0; ++i)
            guess = 0.5 * (guess + x / guess);
        return guess;
    }
    else {
        return std::sqrt(x);
    }
}
static_assert(portable_sqrt(81.0) == 9.0);

// -----------------------------------------------------------------------------
// 2. Calling consteval functions from a constexpr function
// -----------------------------------------------------------------------------
// A consteval checker: rejects overflow at compile time with a clear error.
consteval std::int32_t checked_square(std::int32_t x)
{
    if (x > 46340 || x < -46340)
        throw "square would overflow int32";
    return x * x;
}

// In the `if consteval` branch, x may be passed to a consteval function.
// (With `if (std::is_constant_evaluated())` this was an error - L08-E1.)
constexpr std::int32_t square(std::int32_t x)
{
    if consteval {
        return checked_square(x);   // compile time: checked
    }
    else {
        return x * x;               // runtime: fast, unchecked
    }
}
static_assert(square(1000) == 1000000);

// `if !consteval` reads well for a runtime-only extra.
inline int g_runtimeSquares = 0;
constexpr std::int32_t counted_square(std::int32_t x)
{
    if !consteval {
        ++g_runtimeSquares;
    }
    return x * x;
}
static_assert(counted_square(12) == 144);

// ERROR DEMO [L11-E1]  fails in: every standard
// square(50000) runs the `if consteval` branch, and checked_square rejects it.
#if TUTORIAL_FORCE_ERRORS
constexpr std::int32_t kTooBig = square(50000);
#endif

// ERROR DEMO [L11-E2]  fails in: every standard
// The braces are part of the syntax: `if consteval return 1;` is not valid.
#if TUTORIAL_FORCE_ERRORS
constexpr int no_braces()
{
    if consteval return 1;
    return 0;
}
#endif

// ERROR DEMO [L11-E3]  fails in: every standard
// The ELSE branch is runtime code, so calling the consteval function there
// with the parameter x is still an error.
#if TUTORIAL_FORCE_ERRORS
constexpr std::int32_t wrong_branch(std::int32_t x)
{
    if consteval {
        return x * x;
    }
    else {
        return checked_square(x);
    }
}
#endif

#else
#define L11_HAS_IF_CONSTEVAL 0
#endif // __cpp_if_consteval

// -----------------------------------------------------------------------------
// 3. Consteval propagation (templates and lambdas)
// -----------------------------------------------------------------------------
consteval int triple(int x) { return 3 * x; }

#if defined(__cpp_consteval) && __cpp_consteval >= 202211L
#define L11_HAS_PROPAGATION 1

// A function template calling triple(x) with its own parameter: before
// P2564 this was an error (compare L06-E2). Now sum_tripled<int> simply
// becomes a consteval function.
template <typename T>
constexpr T sum_tripled(T a, T b)
{
    return triple(a) + triple(b);
}
static_assert(sum_tripled(1, 2) == 9);

// Lambdas propagate too.
constexpr auto kTripleTwice = [](int x) { return triple(triple(x)); };
static_assert(kTripleTwice(2) == 18);

// ERROR DEMO [L11-E4]  fails in: every standard
// sum_tripled<int> is now consteval, so calling it with runtime values fails.
#if TUTORIAL_FORCE_ERRORS
inline int error_demo_runtime_call(int a, int b) { return sum_tripled(a, b); }
#endif

// ERROR DEMO [L11-E5]  fails in: every standard
// Propagation only applies to templates and lambdas. A plain constexpr
// function still can't pass its parameter to a consteval function.
#if TUTORIAL_FORCE_ERRORS
constexpr int plain_function(int x) { return triple(x); }
#endif

#else
#define L11_HAS_PROPAGATION 0
#endif // __cpp_consteval >= 202211L

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L11", "if consteval, consteval propagation");

#if L11_HAS_IF_CONSTEVAL
    tut::section(STD, "1. if consteval");
    tut::show(STD, "constexpr auto w = where()", kWhere);
    const char* runtimeWhere = where();
    tut::show(STD, "const char* w = where()", runtimeWhere);
    volatile double two = 2.0;
    tut::show(STD, "portable_sqrt(81) (compile time)", std::integral_constant<int, static_cast<int>(portable_sqrt(81.0))>::value);
    tut::show(STD, "portable_sqrt(2) (runtime)", portable_sqrt(two));

    tut::section(STD, "2. consteval call inside if consteval");
    constexpr std::int32_t checked = square(1000);
    volatile std::int32_t runtimeValue = 3000;
    tut::show(STD, "square(1000) at compile time (checked)", checked);
    tut::show(STD, "square(3000) at runtime (unchecked)", square(runtimeValue));
    tut::note(STD, "square(50000) at compile time is rejected - see ERROR DEMO [L11-E1]");
    volatile std::int32_t twelve = 12;
    tut::show(STD, "counted_square(12) at runtime", counted_square(twelve));
    tut::show(STD, "g_runtimeSquares", g_runtimeSquares);
#else
    tut::note(STD, "`if consteval` is not supported by this compiler (__cpp_if_consteval)");
#endif

    tut::section(STD, "3. consteval propagation");
#if L11_HAS_PROPAGATION
    tut::show(STD, "sum_tripled(1, 2)", sum_tripled(1, 2));
    tut::show(STD, "kTripleTwice(2)", kTripleTwice(2));
#else
    tut::note(STD, "not supported by this compiler (needs __cpp_consteval >= 202211L)");
    tut::note(STD, "e.g. GCC 14+, Clang 17+");
#endif
}

} // namespace l11_cpp23_if_consteval

#endif // TUTORIAL_INCLUDE(23)
