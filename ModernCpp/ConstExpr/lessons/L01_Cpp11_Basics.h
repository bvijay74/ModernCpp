// =============================================================================
// L01 [C++11]  constexpr basics: variables and single-return functions
// =============================================================================
// What C++11 introduced:
//   * constexpr variables  - value computed at compile time
//   * constexpr functions  - CAN be evaluated at compile time if the arguments
//                            are constants; otherwise they run normally
//
// The strict C++11 rules for a constexpr function body:
//   * exactly ONE return statement (plus static_assert / typedef / using)
//   * no local variables, no loops, no if/switch statements
//   * so: use the ?: operator for branches and recursion for loops
//   (C++14 relaxes all of this - see L03.)
//
// Every static_assert in this file is a PROOF: if the program compiles, the
// compiler evaluated that expression during compilation.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(11)

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>

namespace l01_cpp11_basics {

const int STD = 11;

// -----------------------------------------------------------------------------
// 1. constexpr variables vs const variables
// -----------------------------------------------------------------------------
// const      "I won't modify this."  The value may only be known at runtime.
// constexpr  "This value is known at compile time."  (It is also const.)

constexpr int kBufferSize = 64;
static_assert(kBufferSize == 64, "constexpr variables are compile-time constants");

// Old C++98 rule: a const INTEGER initialised with a constant is also usable at
// compile time. That rule only covers integers and enums...
const int kLegacyConst = 32;
static_assert(kLegacyConst == 32, "const int + literal initialiser is a constant expression");

// ...so a const double is NOT a constant expression, but a constexpr double is.
const double kConstDouble = 1.5;
constexpr double kConstexprDouble = 1.5;
static_assert(kConstexprDouble > 1.0, "constexpr works for every literal type");

// ERROR DEMO [L01-E1]  fails in: every standard
// A constexpr variable must be initialised.
// MSVC C2737 'constexpr' object must be initialized
#if TUTORIAL_FORCE_ERRORS
constexpr int n;
#endif

// ERROR DEMO [L01-E2]  fails in: every standard
// The initialiser must be a constant expression. `inc` is an ordinary variable,
// so its value is only known at runtime. Make it `const int inc = 5;` or
// `constexpr int inc = 5;` and the error goes away.
// MSVC C2131 expression did not evaluate to a constant
#if TUTORIAL_FORCE_ERRORS
int inc = 5;
constexpr int x = 100 + inc;
#endif

// ERROR DEMO [L01-E3]  fails in: every standard
// Only const INTEGERS get the old C++98 "usable at compile time" rule;
// a const double does not.
#if TUTORIAL_FORCE_ERRORS
static_assert(kConstDouble > 1.0, "");
#endif

// A value only known at runtime.
inline int runtime_value() { return std::rand() % 10 + 1; }

// ERROR DEMO [L01-E4]  fails in: every standard
// runtime_value() is not a constexpr function.
#if TUTORIAL_FORCE_ERRORS
constexpr int kFromRand = runtime_value();
#endif

// -----------------------------------------------------------------------------
// 2. constexpr functions - C++11 style: one return statement
// -----------------------------------------------------------------------------
constexpr int square(int x) { return x * x; }

// Branch with ?:, loop with recursion.
constexpr unsigned long long factorial(unsigned n)
{
    return n <= 1 ? 1ULL : n * factorial(n - 1);
}

constexpr unsigned long long fibonacci(unsigned n)
{
    return n < 2 ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

constexpr int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }

constexpr double power(double base, unsigned exp)
{
    return exp == 0 ? 1.0 : base * power(base, exp - 1);
}

// A compile-time strlen: string literals are constant expressions.
constexpr std::size_t length(const char* s, std::size_t i = 0)
{
    return s[i] == '\0' ? i : length(s, i + 1);
}

constexpr int count_bits(unsigned v) { return v == 0 ? 0 : int(v & 1u) + count_bits(v >> 1); }

static_assert(square(7) == 49, "");
static_assert(factorial(10) == 3628800ULL, "");
static_assert(fibonacci(20) == 6765ULL, "");
static_assert(gcd(48, 36) == 12, "");
static_assert(power(2.0, 10) == 1024.0, "");
static_assert(length("constexpr") == 9, "");
static_assert(count_bits(0xFFu) == 8, "");

// ERROR DEMO [L01-E5]  fails in: every standard
// Signed overflow is undefined behaviour. At runtime it silently gives garbage;
// during constant evaluation the compiler must reject it - constexpr is a free
// undefined-behaviour detector. (10^10 doesn't fit in an int.)
#if TUTORIAL_FORCE_ERRORS
constexpr int kOverflow = square(100000);
#endif

// ERROR DEMO [L01-E6]  fails in: C++11 only (valid from C++14 - see L03)
// A loop and a local variable in a constexpr function.
// GCC -std=c++11: "body of constexpr function not a return-statement"
// MSVC has no C++11 mode, so in Visual Studio this demo always compiles.
#if TUTORIAL_FORCE_ERRORS
constexpr int sum_to(int n)
{
    int s = 0;
    for (int i = 1; i <= n; ++i)
        s += i;
    return s;
}
#endif

// -----------------------------------------------------------------------------
// 3. Places where the language REQUIRES a constant expression
// -----------------------------------------------------------------------------
// array bounds, template arguments, enumerator values, case labels, static_assert

enum class Limits : int { MaxNotes = square(10) };   // enumerator value
static_assert(static_cast<int>(Limits::MaxNotes) == 100, "");

typedef std::integral_constant<unsigned long long, factorial(12)> Fact12;   // template argument
static_assert(Fact12::value == 479001600ULL, "");

inline const char* classify(int x)
{
    switch (x) {
    case square(2): return "four";   // case labels must be constants
    case square(3): return "nine";
    default:        return "other";
    }
}

// -----------------------------------------------------------------------------
// 4. Errors at compile time: "throw" in a constexpr function
// -----------------------------------------------------------------------------
// C++11 can't have if-statements, but a ?: branch may throw. If the throwing
// branch is reached during constant evaluation, compilation FAILS.
constexpr int checked_divide(int a, int b)
{
    return b == 0 ? throw std::invalid_argument("division by zero") : a / b;
}
static_assert(checked_divide(10, 2) == 5, "");

// ERROR DEMO [L01-E7]  fails in: every standard
// Constant evaluation reaches the `throw` in checked_divide.
#if TUTORIAL_FORCE_ERRORS
static_assert(checked_divide(1, 0) == 0, "");
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L01", "constexpr basics");

    tut::section(STD, "1. constexpr vs const");
    tut::show(STD, "kBufferSize (constexpr int)", kBufferSize);
    tut::show(STD, "kLegacyConst (const int)", kLegacyConst);
    tut::show(STD, "kConstDouble (const, runtime only)", kConstDouble);
    tut::show(STD, "kConstexprDouble (constexpr)", kConstexprDouble);

    const int fromRuntime = runtime_value();   // OK: const may be a runtime value
    // constexpr int bad = runtime_value();  -> see ERROR DEMO [L01-E4]
    tut::show(STD, "const int from runtime_value()", fromRuntime);

    tut::section(STD, "2. constexpr functions (single return)");
    tut::show(STD, "square(7)", square(7));
    tut::show(STD, "factorial(10)", factorial(10));
    tut::show(STD, "fibonacci(20)", fibonacci(20));
    tut::show(STD, "gcd(48, 36)", gcd(48, 36));
    tut::show(STD, "power(2.0, 10)", power(2.0, 10));
    tut::show(STD, "length(\"constexpr\")", length("constexpr"));
    tut::show(STD, "count_bits(0xFF)", count_bits(0xFFu));

    tut::section(STD, "3. constant expressions required by the language");
    int table[square(4)] = {};   // array bound
    tut::show(STD, "int table[square(4)] -> elements", sizeof(table) / sizeof(table[0]));
    tut::show(STD, "Limits::MaxNotes (enum = square(10))", static_cast<int>(Limits::MaxNotes));
    tut::show(STD, "integral_constant<.., factorial(12)>", Fact12::value);
    tut::show(STD, "classify(9) (case square(3))", classify(9));

    tut::section(STD, "4. the SAME function at runtime");
    // volatile stops the compiler from treating n as a known constant.
    volatile unsigned n = 6;
    tut::show(STD, "factorial(n) with runtime n = 6", factorial(n));
    // Assigning to a constexpr variable FORCES compile-time evaluation.
    constexpr unsigned long long forced = factorial(20);
    tut::show(STD, "constexpr forced = factorial(20)", forced);
    tut::note(STD, "constexpr on a function means 'may' run at compile time, not 'must'.");

    tut::section(STD, "5. throw -> compile error, or exception at runtime");
    tut::show(STD, "checked_divide(10, 2)", checked_divide(10, 2));
    try {
        volatile int zero = 0;
        tut::show(STD, "checked_divide(1, zero)", checked_divide(1, zero));
    }
    catch (const std::invalid_argument& e) {
        tut::show(STD, "checked_divide(1, zero) threw", e.what());
    }
}

} // namespace l01_cpp11_basics

#endif // TUTORIAL_INCLUDE(11)
