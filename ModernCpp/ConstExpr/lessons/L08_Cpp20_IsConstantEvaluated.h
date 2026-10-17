// =============================================================================
// L08 [C++20]  std::is_constant_evaluated(): one function, two code paths
// =============================================================================
// std::is_constant_evaluated()   (header <type_traits>)
//   returns true  when the call is part of constant evaluation (compile time)
//   returns false when the function is running normally (runtime)
//
// Use it when the best runtime implementation can't be constexpr (a CPU
// intrinsic, std::sqrt, logging, a lookup in a runtime cache...) but you still
// want the function to work at compile time.
//
// PITFALL: never write  if constexpr (std::is_constant_evaluated())
// The condition of `if constexpr` is ALWAYS evaluated at compile time, so it is
// always true and the runtime branch is thrown away. Compilers warn about it.
// Use a plain `if`, or `if consteval` in C++23 (L11).
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <cmath>
#include <type_traits>

namespace l08_cpp20_is_constant_evaluated {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. Where am I being evaluated?
// -----------------------------------------------------------------------------
constexpr const char* where() { return std::is_constant_evaluated() ? "compile time" : "runtime"; }

constexpr const char* kWhereConstexpr = where();   // constexpr variable -> compile time

// -----------------------------------------------------------------------------
// 2. Different implementations for compile time and runtime
// -----------------------------------------------------------------------------
// std::sqrt isn't constexpr until C++26, so at compile time use Newton's method.
constexpr double portable_sqrt(double x)
{
    if (std::is_constant_evaluated()) {
        if (x <= 0)
            return 0;
        double guess = x;
        for (int i = 0; i < 64; ++i)
            guess = 0.5 * (guess + x / guess);
        return guess;
    }
    return std::sqrt(x);   // runtime: the fast library function
}

static_assert(portable_sqrt(16.0) == 4.0);
static_assert(portable_sqrt(2.0) > 1.41421 && portable_sqrt(2.0) < 1.41422);

// -----------------------------------------------------------------------------
// 3. Runtime-only side effects
// -----------------------------------------------------------------------------
// The runtime branch may touch things that aren't allowed at compile time,
// like a mutable global. That code is simply never reached during constant
// evaluation.
inline int g_runtimeCalls = 0;

constexpr int clamp_velocity(int v)
{
    if (!std::is_constant_evaluated())
        ++g_runtimeCalls;   // only counted at runtime
    return v < 0 ? 0 : (v > 127 ? 127 : v);
}
static_assert(clamp_velocity(200) == 127);

// ERROR DEMO [L08-E1]  fails in: every standard
// is_constant_evaluated() is an ordinary function returning bool, so the
// compiler still checks the "true" branch as if it could run at runtime.
// Calling a consteval function with the parameter x is therefore still an
// error. This is exactly what C++23's `if consteval` fixes (L11).
#if TUTORIAL_FORCE_ERRORS
consteval int checked_square(int x) { return x * x; }
constexpr int square(int x)
{
    if (std::is_constant_evaluated())
        return checked_square(x);
    return x * x;
}
#endif

// ERROR DEMO [L08-E2]  fails in: C++20 (valid from C++23 - see L12)
// A static constexpr variable inside a constexpr function. C++20 forbids any
// static local in a constexpr function, so the table must live outside it.
// (Clang accepts it in C++20 mode with a warning.)
#if TUTORIAL_FORCE_ERRORS
constexpr int semitone_offset(int degree)
{
    static constexpr int kMajorScale[] = {0, 2, 4, 5, 7, 9, 11};
    return kMajorScale[degree % 7];
}
#endif

// -----------------------------------------------------------------------------
// 4. When does a call count as "constant evaluated"?
// -----------------------------------------------------------------------------
// It is about the CONTEXT of the call, not the arguments:
//   constexpr auto a = f();          yes - constexpr variable
//   static_assert(f());              yes
//   std::array<int, f()> arr;        yes - template argument
//   const int b = f();               yes - const integer with constant initialiser
//   int c = f();                     no  - ordinary variable (even with constant args)
//   auto d = f();  (pointer, double) no

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L08", "std::is_constant_evaluated()");

    tut::section(STD, "1. where am I evaluated?");
    tut::show(STD, "constexpr auto w = where()", kWhereConstexpr);
    const char* runtimeWhere = where();   // plain variable -> runtime
    tut::show(STD, "const char* w = where()", runtimeWhere);

    tut::section(STD, "2. compile-time Newton vs runtime std::sqrt");
    constexpr double ct = portable_sqrt(2.0);
    volatile double two = 2.0;
    const double rt = portable_sqrt(two);
    tut::show(STD, "portable_sqrt(2) at compile time", ct);
    tut::show(STD, "portable_sqrt(2) at runtime", rt);

    tut::section(STD, "3. runtime-only side effects");
    constexpr int ctVelocity = clamp_velocity(200);   // not counted
    volatile int raw = -5;
    const int rtVelocity = clamp_velocity(raw);        // counted
    tut::show(STD, "clamp_velocity(200) (compile time)", ctVelocity);
    tut::show(STD, "clamp_velocity(-5) (runtime)", rtVelocity);
    tut::show(STD, "g_runtimeCalls", g_runtimeCalls);
}

} // namespace l08_cpp20_is_constant_evaluated

#endif // TUTORIAL_INCLUDE(20)
