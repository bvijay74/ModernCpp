// =============================================================================
// L03 [C++14]  relaxed constexpr: functions that look like normal code
// =============================================================================
// What C++14 added (paper N3652):
//   * local variables (must be initialised)
//   * loops: for, while, do-while, range-for
//   * if and switch statements, multiple return statements
//   * modifying objects whose lifetime began during the constant evaluation
//   * constexpr member functions are NO LONGER implicitly const
//   * constexpr functions may return void
// Also new in C++14 and handy here:
//   * variable templates            template <class T> constexpr T pi = ...;
//   * binary literals, digit separators  0b1111'0000
//   * std::max, std::min, std::array::operator[] const, std::get are constexpr
//
// Still NOT allowed in C++14 constexpr functions:
//   uninitialised locals, static/thread_local locals, goto, try blocks,
//   virtual calls, new/delete, non-literal local variables.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(14)

#include <algorithm>
#include <array>
#include <cstddef>

namespace l03_cpp14_relaxed {

const int STD = 14;

// -----------------------------------------------------------------------------
// 1. Side by side: C++11 style vs C++14 style
// -----------------------------------------------------------------------------
constexpr unsigned long long factorial_11(unsigned n)   // C++11: recursion + ?:
{
    return n <= 1 ? 1ULL : n * factorial_11(n - 1);
}

constexpr unsigned long long factorial_14(unsigned n)   // C++14: plain loop
{
    unsigned long long result = 1;
    for (unsigned i = 2; i <= n; ++i)
        result *= i;
    return result;
}
static_assert(factorial_11(15) == factorial_14(15), "both give the same answer");

// The C++11 recursive Fibonacci is exponential; the loop version is linear,
// so the compiler can easily do fibonacci(90).
constexpr unsigned long long fibonacci(unsigned n)
{
    unsigned long long a = 0, b = 1;
    while (n-- > 0) {
        unsigned long long next = a + b;
        a = b;
        b = next;
    }
    return a;
}
static_assert(fibonacci(90) == 2880067194370816120ULL, "");

// -----------------------------------------------------------------------------
// 2. if, early return, switch
// -----------------------------------------------------------------------------
constexpr bool is_prime(unsigned n)
{
    if (n < 2)
        return false;
    for (unsigned d = 2; d * d <= n; ++d)
        if (n % d == 0)
            return false;
    return true;
}
static_assert(is_prime(97) && !is_prime(91), "");

constexpr int days_in_month(int month, bool leap)
{
    switch (month) {
    case 2:
        return leap ? 29 : 28;
    case 4: case 6: case 9: case 11:
        return 30;
    default:
        return 31;
    }
}
static_assert(days_in_month(2, true) == 29, "");

constexpr int count_vowels(const char* s)
{
    int count = 0;
    for (; *s != '\0'; ++s) {
        switch (*s) {
        case 'a': case 'e': case 'i': case 'o': case 'u':
            ++count;
            break;
        default:
            break;
        }
    }
    return count;
}
static_assert(count_vowels("compile time evaluation") == 11, "");

// Range-for over a C array.
template <std::size_t N>
constexpr int sum(const int (&values)[N])
{
    int total = 0;
    for (int v : values)
        total += v;
    return total;
}
constexpr int kMidiChord[] = {60, 64, 67, 72};
static_assert(sum(kMidiChord) == 263, "");

// ERROR DEMO [L03-E1]  fails in: every standard
// Division by zero during constant evaluation (kMidiChord[0] is 60).
#if TUTORIAL_FORCE_ERRORS
constexpr int kDivZero = 100 / (kMidiChord[0] - 60);
#endif

// ERROR DEMO [L03-E2]  fails in: every standard
// A constexpr function may contain non-constexpr code on paths that are not
// taken. Only the path actually evaluated has to be constant.
#if TUTORIAL_FORCE_ERRORS
inline int not_constexpr() { return 7; }
constexpr int pick(int v)
{
    if (v >= 0)
        return v;
    return not_constexpr();
}
static_assert(pick(5) == 5, "OK: the non-constexpr path isn't reached");
constexpr int kBadPick = pick(-1);   // error: this call reaches not_constexpr()
#endif

// -----------------------------------------------------------------------------
// 3. Mutation: constexpr member functions are no longer implicitly const
// -----------------------------------------------------------------------------
class Counter {
public:
    constexpr Counter() : count_(0) {}
    constexpr void increment() { ++count_; }      // C++14: mutating + void return
    constexpr int value() const { return count_; }

private:
    int count_;
};

// A local object is created, modified and read - all at compile time.
constexpr int count_multiples(int limit, int k)
{
    Counter c;
    for (int i = 1; i <= limit; ++i)
        if (i % k == 0)
            c.increment();
    return c.value();
}
static_assert(count_multiples(100, 7) == 14, "");

// ERROR DEMO [L03-E3]  fails in: every standard
// Mutation is allowed only on objects created DURING the evaluation.
// A constexpr object is const forever.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_counter()
{
    constexpr Counter kCounter;
    kCounter.increment();   // increment() is not const
}
#endif

// -----------------------------------------------------------------------------
// 4. Building a lookup table at compile time
// -----------------------------------------------------------------------------
// In C++14 std::array's NON-const operator[] is not constexpr yet (C++17), so
// we wrap a plain C array in our own struct.
struct SquareTable {
    int values[16];
};

constexpr SquareTable make_square_table()
{
    SquareTable t{};             // must be initialised in C++14 ("SquareTable t;" is an error)
    for (int i = 0; i < 16; ++i)
        t.values[i] = i * i;
    return t;
}
constexpr SquareTable kSquares = make_square_table();
static_assert(kSquares.values[15] == 225, "");

// ERROR DEMO [L03-E4]  fails in: every standard
// Reading past the end of an array is undefined behaviour (valid indexes are 0..15).
#if TUTORIAL_FORCE_ERRORS
constexpr int kPastEnd = kSquares.values[16];
#endif

// ERROR DEMO [L03-E5]  fails in: C++14 only (valid from C++17 - see L05)
// std::array's NON-const operator[] isn't constexpr until C++17. The error only
// appears when the function is used in a constant expression.
#if TUTORIAL_FORCE_ERRORS
constexpr std::array<int, 4> make_array()
{
    std::array<int, 4> a{};
    a[0] = 1;
    return a;
}
constexpr std::array<int, 4> kMadeArray = make_array();
#endif

// -----------------------------------------------------------------------------
// 5. Variable templates (C++14)
// -----------------------------------------------------------------------------
template <typename T>
constexpr T pi = T(3.1415926535897932385L);

template <typename T>
constexpr T two_pi = 2 * pi<T>;

template <unsigned N>
constexpr unsigned long long factorial_v = factorial_14(N);   // like a type trait's ::value

static_assert(pi<int> == 3, "");
static_assert(factorial_v<5> == 120, "");

// -----------------------------------------------------------------------------
// 6. Binary literals, digit separators, standard library
// -----------------------------------------------------------------------------
constexpr unsigned kHighNibble = 0b1111'0000;
constexpr long kOneMillion = 1'000'000;
static_assert(kHighNibble == 0xF0 && kOneMillion == 1000000, "");

static_assert(std::max(3, 7) == 7, "std::max is constexpr since C++14");
constexpr std::array<int, 3> kTriad = {{60, 64, 67}};
static_assert(kTriad[2] == 67 && std::get<0>(kTriad) == 60, "const operator[] / std::get");

// Still forbidden in C++14:
// ERROR DEMO [L03-E6]  fails in: C++14, C++17 (valid from C++20)
// An uninitialised local variable. (Clang accepts it in C++14/17 with a warning.)
#if TUTORIAL_FORCE_ERRORS
constexpr int uninitialised_local() { int x; x = 1; return x; }
#endif

// A try block is also forbidden in C++14/17 (allowed from C++20), but GCC and
// Clang only warn about it, so it isn't an error demo here.

// ERROR DEMO [L03-E7]  fails in: C++14 ... C++20 (valid from C++23, see L12)
// A static local variable. C++23 allows it as long as the function is never
// evaluated at compile time. Compiler support varies: GCC 13 accepts it in
// C++23 mode, Clang 18 still rejects it.
#if TUTORIAL_FORCE_ERRORS
constexpr int with_static() { static int calls = 0; return ++calls; }
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L03", "relaxed constexpr");

    tut::section(STD, "1. C++11 style vs C++14 style");
    tut::show(STD, "factorial_11(15) (recursive)", factorial_11(15));
    tut::show(STD, "factorial_14(15) (loop)", factorial_14(15));
    tut::show(STD, "fibonacci(90) (loop)", fibonacci(90));

    tut::section(STD, "2. if / switch / loops");
    tut::show(STD, "is_prime(97)", is_prime(97));
    tut::show(STD, "days_in_month(2, leap)", days_in_month(2, true));
    tut::show(STD, "count_vowels(\"compile time evaluation\")", count_vowels("compile time evaluation"));
    tut::show(STD, "sum({60, 64, 67, 72})", sum(kMidiChord));

    tut::section(STD, "3. mutation inside constant evaluation");
    tut::show(STD, "count_multiples(100, 7)", count_multiples(100, 7));

    tut::section(STD, "4. compile-time lookup table");
    std::string row;
    for (int v : kSquares.values)
        row += std::to_string(v) + ' ';
    tut::show(STD, "kSquares", row);

    tut::section(STD, "5. variable templates");
    tut::show(STD, "pi<int>", pi<int>);
    tut::show(STD, "pi<float>", pi<float>);
    tut::show(STD, "two_pi<double>", two_pi<double>);
    tut::show(STD, "factorial_v<10>", factorial_v<10>);

    tut::section(STD, "6. literals and library");
    tut::show(STD, "0b1111'0000", kHighNibble);
    tut::show(STD, "1'000'000", kOneMillion);

    tut::section(STD, "runtime use of the same functions");
    volatile unsigned n = 89;
    unsigned primesBelow = 0;
    for (unsigned i = 0; i <= n; ++i)
        if (is_prime(i))
            ++primesBelow;
    tut::show(STD, "primes <= 89 (counted at runtime)", primesBelow);
}

} // namespace l03_cpp14_relaxed

#endif // TUTORIAL_INCLUDE(14)
