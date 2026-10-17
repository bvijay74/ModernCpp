// =============================================================================
// L04 [C++17]  if constexpr: choosing code at compile time
// =============================================================================
// if constexpr (condition) { A } else { B }
//   * condition must be a constant expression
//   * inside a TEMPLATE, the branch not taken is DISCARDED: it isn't
//     instantiated, so it may contain code that wouldn't compile for this T
//     (e.g. *value when T isn't a pointer)
//   * different branches may return different types from an `auto` function
//
// It replaces most uses of tag dispatch, SFINAE overloads and the
// "recursion terminator overload" in variadic templates.
//
// Also new in C++17 and used here: the _v trait shortcuts (std::is_integral_v),
// fold expressions, and static_assert without a message.
//
// MSVC NOTE: in /std:c++14 mode MSVC accepts `if constexpr` with only
// warning C4984 ("'if constexpr' is a C++17 language extension"). The rest of
// this lesson (_v traits, fold expressions, static_assert without message)
// still produces real errors.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(17)

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace l04_cpp17_if_constexpr {

const int STD = 17;

// -----------------------------------------------------------------------------
// 1. The old way: tag dispatch (C++11/14) - one overload per case
// -----------------------------------------------------------------------------
namespace old_way {
template <typename T>
std::string describe_impl(const T& v, std::true_type /*is integral*/)
{
    return "integer " + std::to_string(v);
}
template <typename T>
std::string describe_impl(const T&, std::false_type /*not integral*/)
{
    return "not an integer";
}
template <typename T>
std::string describe(const T& v)
{
    return describe_impl(v, std::is_integral<T>{});
}
} // namespace old_way

// -----------------------------------------------------------------------------
// 2. The new way: one function, if constexpr chain
// -----------------------------------------------------------------------------
template <typename T>
std::string describe(const T& value)
{
    if constexpr (std::is_same_v<T, bool>)
        return value ? "bool true" : "bool false";
    else if constexpr (std::is_same_v<T, char>)
        return std::string("char '") + value + "'";
    else if constexpr (std::is_integral_v<T>)
        return "integer " + std::to_string(value);   // to_string(T) only compiled for integers
    else if constexpr (std::is_floating_point_v<T>)
        return "floating " + std::to_string(value);
    else if constexpr (std::is_convertible_v<T, std::string>)
        return "string \"" + std::string(value) + "\"";
    else if constexpr (std::is_pointer_v<T>)
        return value ? "pointer to " + describe(*value) : "null pointer";   // *value only for pointers
    else
        return "unknown type";
}

// -----------------------------------------------------------------------------
// 3. Discarded branches + different return types (constexpr function)
// -----------------------------------------------------------------------------
template <typename T>
constexpr auto unwrap(T v)
{
    if constexpr (std::is_pointer_v<T>)
        return *v;   // with a plain `if`, this wouldn't compile for T = int
    else
        return v;
}

constexpr int kAnswer = 42;
static_assert(unwrap(kAnswer) == 42);
static_assert(unwrap(&kAnswer) == 42);

// Picking a TYPE at compile time.
template <std::size_t Bits>
constexpr auto smallest_uint()
{
    static_assert(Bits <= 64, "at most 64 bits");
    if constexpr (Bits <= 8)
        return std::uint8_t{};
    else if constexpr (Bits <= 16)
        return std::uint16_t{};
    else if constexpr (Bits <= 32)
        return std::uint32_t{};
    else
        return std::uint64_t{};
}
template <std::size_t Bits>
using uint_least = decltype(smallest_uint<Bits>());

static_assert(std::is_same_v<uint_least<7>, std::uint8_t>);   // C++17: no message needed
static_assert(std::is_same_v<uint_least<12>, std::uint16_t>);
static_assert(std::is_same_v<uint_least<33>, std::uint64_t>);

// -----------------------------------------------------------------------------
// 4. Variadic templates without the terminator overload
// -----------------------------------------------------------------------------
// C++11/14 needed two overloads: sum(T) and sum(T, Rest...).
template <typename T, typename... Rest>
constexpr auto sum_recursive(T first, Rest... rest)
{
    if constexpr (sizeof...(rest) == 0)
        return first;
    else
        return first + sum_recursive(rest...);
}

// C++17 fold expression: no recursion at all.
template <typename... Ts>
constexpr auto sum_fold(Ts... xs)
{
    return (xs + ...);
}

static_assert(sum_recursive(1, 2, 3, 4) == 10);
static_assert(sum_fold(1, 2, 3, 4) == 10);
static_assert(sum_fold(0.5, 1, 2) == 3.5);

// -----------------------------------------------------------------------------
// 5. A tuple-like get<I>() for your own struct
// -----------------------------------------------------------------------------
struct PianoKey {
    int number;
    double frequency;
    const char* name;
};

template <std::size_t I>
constexpr auto get(const PianoKey& k)
{
    if constexpr (I == 0)
        return k.number;
    else if constexpr (I == 1)
        return k.frequency;
    else {
        static_assert(I == 2, "PianoKey has only 3 fields");   // depends on I -> only fires if reached
        return k.name;
    }
}

constexpr PianoKey kA4{49, 440.0, "A4"};
static_assert(get<0>(kA4) == 49);
static_assert(get<1>(kA4) == 440.0);
// ERROR DEMO [L04-E1]  fails in: every standard
// get<3> reaches the static_assert in the final else: "PianoKey has only 3 fields".
#if TUTORIAL_FORCE_ERRORS
constexpr auto kNoSuchField = get<3>(kA4);
#endif

// -----------------------------------------------------------------------------
// Pitfalls
// -----------------------------------------------------------------------------
// ERROR DEMO [L04-E2]  fails in: every standard
// The if constexpr condition must be a constant expression.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_runtime_condition()
{
    int n = 3;
    if constexpr (n > 2) {
    }
}
#endif
//
// Outside a template, BOTH branches are still fully compiled (nothing is
// "discarded"), so if constexpr is not a replacement for #if.
//
// static_assert(false) in a discarded branch is an error before C++23,
// because it doesn't depend on T. The workaround is a dependent false:
template <typename>
inline constexpr bool dependent_false = false;

template <typename T>
constexpr int bits_of()
{
    if constexpr (std::is_integral_v<T>)
        return sizeof(T) * 8;
    else
        static_assert(dependent_false<T>, "bits_of<T> needs an integral type");
}
static_assert(bits_of<int>() == 32 || bits_of<int>() == 16);

// ERROR DEMO [L04-E3]  fails in: every standard
// bits_of<double> instantiates the dependent_false branch: "bits_of<T> needs an integral type".
#if TUTORIAL_FORCE_ERRORS
constexpr int kBitsOfDouble = bits_of<double>();
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L04", "if constexpr");

    tut::section(STD, "1. old way: tag dispatch (C++11/14)");
    tut::show(STD, "old_way::describe(7)", old_way::describe(7));
    tut::show(STD, "old_way::describe(2.5)", old_way::describe(2.5));

    tut::section(STD, "2. new way: if constexpr chain");
    int key = 49;
    int* keyPtr = &key;
    tut::show(STD, "describe(true)", describe(true));
    tut::show(STD, "describe('A')", describe('A'));
    tut::show(STD, "describe(49)", describe(49));
    tut::show(STD, "describe(440.0)", describe(440.0));
    tut::show(STD, "describe(\"A4\")", describe("A4"));
    tut::show(STD, "describe(int* -> 49)", describe(keyPtr));
    tut::show(STD, "describe(nullptr int*)", describe(static_cast<int*>(nullptr)));

    tut::section(STD, "3. discarded branches / type selection");
    tut::show(STD, "unwrap(&kAnswer)", unwrap(&kAnswer));
    tut::show(STD, "sizeof(uint_least<12>)", sizeof(uint_least<12>));
    tut::show(STD, "sizeof(uint_least<33>)", sizeof(uint_least<33>));

    tut::section(STD, "4. variadic sum");
    tut::show(STD, "sum_recursive(1, 2, 3, 4)", sum_recursive(1, 2, 3, 4));
    tut::show(STD, "sum_fold(0.5, 1, 2)", sum_fold(0.5, 1, 2));

    tut::section(STD, "5. get<I>(PianoKey)");
    tut::show(STD, "get<0>(kA4) number", get<0>(kA4));
    tut::show(STD, "get<1>(kA4) frequency", get<1>(kA4));
    tut::show(STD, "get<2>(kA4) name", get<2>(kA4));
    tut::show(STD, "bits_of<int>()", bits_of<int>());
}

} // namespace l04_cpp17_if_constexpr

#endif // TUTORIAL_INCLUDE(17)
