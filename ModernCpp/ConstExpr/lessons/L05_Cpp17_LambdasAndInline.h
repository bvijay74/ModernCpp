// =============================================================================
// L05 [C++17]  constexpr lambdas, inline variables, constexpr library
// =============================================================================
// What C++17 added:
//   * lambdas are implicitly constexpr when their body allows it; you can
//     also write `constexpr` explicitly after the parameter list
//   * inline variables: `inline constexpr` in a header gives ONE object for
//     the whole program; static constexpr data members are implicitly inline
//     (no more out-of-class definition - compare with L02 section 5)
//   * much more of the standard library is constexpr:
//       std::array (non-const operator[], begin/end), std::string_view,
//       std::char_traits, std::size, std::clamp ...
//   * class template argument deduction (CTAD):  std::array a{1, 2, 3};
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(17)

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstddef>
#include <iterator>
#include <string_view>

namespace l05_cpp17_lambdas_inline {

const int STD = 17;

// -----------------------------------------------------------------------------
// 1. constexpr lambdas
// -----------------------------------------------------------------------------
constexpr auto square = [](int x) { return x * x; };            // implicitly constexpr
constexpr auto cube = [](int x) constexpr { return x * x * x; };   // explicitly constexpr:
                                                                   // error if the body can't be
static_assert(square(12) == 144);
static_assert(cube(3) == 27);

// Lambdas (with captures) inside a constexpr function.
constexpr int add_twice(int x, int k)
{
    auto add_k = [k](int v) { return v + k; };
    return add_k(add_k(x));
}
static_assert(add_twice(1, 5) == 11);

// Passing a lambda to a constexpr higher-order function.
template <typename F>
constexpr int accumulate_n(int n, F f)
{
    int total = 0;
    for (int i = 0; i < n; ++i)
        total += f(i);
    return total;
}
static_assert(accumulate_n(5, square) == 30);   // 0 + 1 + 4 + 9 + 16
static_assert(accumulate_n(4, [](int i) { return i * 10; }) == 60);

// NOTE: in C++14 no lambda can be used in a constant expression at all.
// That's one reason this lesson is only compiled from C++17 on.

// ERROR DEMO [L05-E1]  fails in: every standard
// A closure that captures a runtime variable isn't a constant.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_capture()
{
    int runtime = 3;
    constexpr auto add = [runtime](int v) { return v + runtime; };
    (void)add;
}
#endif

// ERROR DEMO [L05-E2]  fails in: every standard
// A lambda is only IMPLICITLY constexpr when its body allows it. This one
// compiles fine (it can still be called at runtime), but calling it in a
// constant expression fails.
#if TUTORIAL_FORCE_ERRORS
constexpr auto add_random = [](int v) { return v + std::rand(); };
static_assert(add_random(1) > 0);
#endif

// ERROR DEMO [L05-E3]  fails in: every standard
// Writing `constexpr` explicitly makes the compiler check the body when the
// lambda is called in a constant expression.
#if TUTORIAL_FORCE_ERRORS
constexpr auto explicit_random = [](int v) constexpr { return v + std::rand(); };
constexpr int kExplicit = explicit_random(1);
#endif

// -----------------------------------------------------------------------------
// 2. Immediately-invoked lambda to build a table (compare L03 section 4)
// -----------------------------------------------------------------------------
// std::array's non-const operator[] is constexpr now, so no wrapper struct.
constexpr auto kSquares = [] {
    std::array<int, 16> a{};
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] = static_cast<int>(i * i);
    return a;
}();
static_assert(kSquares[15] == 225);

// CTAD: element type and size deduced from the initialiser.
constexpr std::array kPrimes{2, 3, 5, 7, 11, 13};
static_assert(kPrimes.size() == 6);
static_assert(std::size(kPrimes) == 6);

constexpr int kPrimeSum = [] {
    int s = 0;
    for (int p : kPrimes)   // begin()/end() are constexpr in C++17
        s += p;
    return s;
}();
static_assert(kPrimeSum == 41);

// -----------------------------------------------------------------------------
// 3. std::string_view at compile time
// -----------------------------------------------------------------------------
constexpr std::string_view kTitle = "constexpr in C++17";
static_assert(kTitle.size() == 18);
static_assert(kTitle.find("C++") == 13);
static_assert(kTitle.substr(0, 9) == "constexpr");

// ERROR DEMO [L05-E4]  fails in: every standard
// A constexpr string_view is const; remove_prefix() modifies it.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_remove_prefix() { kTitle.remove_prefix(1); }
#endif

constexpr std::size_t count_char(std::string_view s, char c)
{
    std::size_t n = 0;
    for (char ch : s)
        if (ch == c)
            ++n;
    return n;
}
static_assert(count_char("hello world", 'o') == 2);

constexpr bool starts_with(std::string_view s, std::string_view prefix)   // std::starts_with is C++20
{
    return s.substr(0, prefix.size()) == prefix;
}
static_assert(starts_with("C#4", "C#"));

// -----------------------------------------------------------------------------
// 4. inline variables
// -----------------------------------------------------------------------------
// A namespace-scope constexpr variable has internal linkage: every .cpp that
// includes this header gets its OWN copy. `inline` makes it one shared object
// (same address everywhere) - the right default for constants in headers.
inline constexpr double kSampleRate = 48000.0;

// Header-only global that isn't constant at all - also only possible since C++17.
inline int g_runCount = 0;

struct Settings {
    static constexpr int kBlockSize = 512;   // implicitly inline in C++17:
};                                           // no out-of-class definition needed

// -----------------------------------------------------------------------------
// 5. More constexpr library: std::clamp
// -----------------------------------------------------------------------------
static_assert(std::clamp(130, 0, 127) == 127);   // e.g. MIDI velocity
static_assert(std::clamp(-5, 0, 127) == 0);

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L05", "constexpr lambdas, inline variables, library");
    ++g_runCount;

    tut::section(STD, "1. constexpr lambdas");
    tut::show(STD, "square(12)", square(12));
    tut::show(STD, "cube(3)", cube(3));
    tut::show(STD, "add_twice(1, 5)", add_twice(1, 5));
    tut::show(STD, "accumulate_n(5, square)", accumulate_n(5, square));

    tut::section(STD, "2. tables via immediately-invoked lambda");
    std::string row;
    for (int v : kSquares)
        row += std::to_string(v) + ' ';
    tut::show(STD, "kSquares", row);
    tut::show(STD, "kPrimes (CTAD) size", kPrimes.size());
    tut::show(STD, "kPrimeSum", kPrimeSum);

    tut::section(STD, "3. string_view");
    tut::show(STD, "kTitle", kTitle);
    tut::show(STD, "kTitle.find(\"C++\")", kTitle.find("C++"));
    tut::show(STD, "count_char(\"hello world\", 'o')", count_char("hello world", 'o'));
    tut::show(STD, "starts_with(\"C#4\", \"C#\")", starts_with("C#4", "C#"));

    tut::section(STD, "4. inline variables");
    tut::show(STD, "kSampleRate", kSampleRate);
    // std::max takes const int& (odr-use) - fine without a definition in C++17
    tut::show(STD, "std::max(kBlockSize, 256)", std::max(Settings::kBlockSize, 256));
    tut::show(STD, "g_runCount", g_runCount);

    tut::section(STD, "5. std::clamp");
    tut::show(STD, "std::clamp(130, 0, 127)", std::clamp(130, 0, 127));
}

} // namespace l05_cpp17_lambdas_inline

#endif // TUTORIAL_INCLUDE(17)
