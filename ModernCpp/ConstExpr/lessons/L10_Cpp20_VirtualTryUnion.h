// =============================================================================
// L10 [C++20]  more of the language at compile time
// =============================================================================
// C++20 also allows these in constexpr functions:
//   * virtual function calls (and constexpr virtual functions)
//   * dynamic_cast and polymorphic typeid
//   * try/catch blocks (but actually THROWING during constant evaluation is
//     still an error - the catch can only run at runtime; C++26 changes this)
//   * changing the active member of a union
//   * uninitialised local variables (int x;) and the asm declaration
// And for templates:
//   * class types and floating-point values as non-type template parameters
//     (NTTPs): template <FixedString Name>, template <double Hz>
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <cstddef>
#include <string_view>
#include <type_traits>

namespace l10_cpp20_virtual_try_union {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. Virtual functions at compile time
// -----------------------------------------------------------------------------
struct Instrument {
    constexpr virtual ~Instrument() {}   // constexpr virtual destructor
    constexpr virtual int lowest_midi_note() const = 0;
    constexpr virtual int highest_midi_note() const = 0;
    constexpr int range() const { return highest_midi_note() - lowest_midi_note() + 1; }
};

// The destructors are written out as {} on purpose: GCC 13 rejects implicit or
// "= default" constexpr virtual destructors when they're called through a base
// pointer at compile time (a compiler bug - the standard allows it).
struct Piano : Instrument {
    constexpr ~Piano() override {}
    constexpr int lowest_midi_note() const override { return 21; }    // A0
    constexpr int highest_midi_note() const override { return 108; }  // C8
};

struct Violin : Instrument {
    constexpr ~Violin() override {}
    constexpr int lowest_midi_note() const override { return 55; }    // G3
    constexpr int highest_midi_note() const override { return 103; }
};

// Polymorphism through base pointers, with new/delete (L09), all at compile time.
constexpr int total_range()
{
    Instrument* band[] = {new Piano, new Violin};
    int total = 0;
    for (Instrument* i : band)
        total += i->range();   // virtual call
    for (Instrument* i : band)
        delete i;              // virtual destructor
    return total;
}
static_assert(Piano{}.range() == 88);
static_assert(total_range() == 88 + 49);

// dynamic_cast in a constant expression.
constexpr bool is_piano(const Instrument& i) { return dynamic_cast<const Piano*>(&i) != nullptr; }
constexpr bool kDynamicCastWorks = [] {
    Piano p;
    Violin v;
    return is_piano(p) && !is_piano(v);
}();
static_assert(kDynamicCastWorks);

// ERROR DEMO [L10-E1]  fails in: every standard
// dynamic_cast to a REFERENCE throws std::bad_cast when the object has a
// different type - and throwing during constant evaluation is an error.
#if TUTORIAL_FORCE_ERRORS
constexpr int violin_as_piano()
{
    Violin v;
    const Instrument& i = v;
    return dynamic_cast<const Piano&>(i).range();
}
static_assert(violin_as_piano() == 88);
#endif

// -----------------------------------------------------------------------------
// 2. try/catch in a constexpr function
// -----------------------------------------------------------------------------
// The try block is allowed. At compile time nothing may throw, so the catch is
// only ever used at runtime.
inline int g_parseFailures = 0;

constexpr int parse_digit(char c)
{
    try {
        if (c < '0' || c > '9')
            throw c;   // fine as long as constant evaluation never gets here
        return c - '0';
    }
    catch (char) {
        ++g_parseFailures;
        return -1;
    }
}
static_assert(parse_digit('7') == 7);

// ERROR DEMO [L10-E2]  fails in: C++20, C++23 (C++26 allows it)
// Constant evaluation reaches a throw. The surrounding try/catch doesn't help:
// before C++26, exceptions can't be thrown or caught at compile time.
#if TUTORIAL_FORCE_ERRORS
static_assert(parse_digit('x') == -1);
#endif

// -----------------------------------------------------------------------------
// 3. Unions: changing the active member
// -----------------------------------------------------------------------------
union Sample {
    int asInt;
    float asFloat;
};

constexpr float switch_member()
{
    Sample s{.asInt = 42};   // designated initialiser (C++20): asInt is active
    s.asFloat = 0.5f;        // C++20: assignment makes asFloat the active member
    return s.asFloat;
}
static_assert(switch_member() == 0.5f);

// ERROR DEMO [L10-E3]  fails in: every standard
// Reading a member that is NOT active ("type punning") is undefined behaviour.
#if TUTORIAL_FORCE_ERRORS
constexpr int read_inactive()
{
    Sample s{.asFloat = 1.0f};
    return s.asInt;
}
static_assert(read_inactive() != 0);
#endif

// Uninitialised locals are allowed now - as long as you assign before reading.
constexpr int late_init(bool high)
{
    int note;   // C++20: OK without an initialiser
    if (high)
        note = 96;
    else
        note = 36;
    return note;
}
static_assert(late_init(true) == 96);

// -----------------------------------------------------------------------------
// 4. Class types as non-type template parameters
// -----------------------------------------------------------------------------
// A "structural" type (public members, no mutable, all members structural)
// can be a template parameter. A fixed-size string lets you write
// Patch<"Grand Piano">.
template <std::size_t N>
struct FixedString {
    char text[N]{};
    constexpr FixedString(const char (&s)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            text[i] = s[i];
    }
    constexpr std::string_view view() const { return {text, N - 1}; }
};

template <FixedString Name, int Program>
struct Patch {
    static constexpr std::string_view name = Name.view();
    static constexpr int program = Program;
};

using GrandPiano = Patch<"Grand Piano", 0>;
using Strings = Patch<"Strings", 48>;
static_assert(GrandPiano::name == "Grand Piano");
static_assert(Strings::program == 48);

// ERROR DEMO [L10-E4]  fails in: every standard
// A class with a PRIVATE data member is not structural, so it can't be a
// template parameter.
#if TUTORIAL_FORCE_ERRORS
class Hidden {
public:
    constexpr Hidden(int v) : value_(v) {}

private:
    int value_;
};
template <Hidden H>
struct UsesHidden {};
using Bad = UsesHidden<Hidden{1}>;
#endif

// Floating-point template parameters (C++20).
#if defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911L
#define L10_HAS_FLOAT_NTTP 1
template <double Hz>
struct Tone {
    static constexpr double frequency = Hz;
    static constexpr double period_ms = 1000.0 / Hz;
};
using A4 = Tone<440.0>;
static_assert(A4::period_ms > 2.27 && A4::period_ms < 2.28);
#else
#define L10_HAS_FLOAT_NTTP 0
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L10", "virtual, try/catch, unions, class-type NTTPs");

    tut::section(STD, "1. virtual functions at compile time");
    constexpr int pianoRange = Piano{}.range();
    tut::show(STD, "Piano{}.range()", pianoRange);
    tut::show(STD, "total_range() (Piano + Violin)", total_range());
    tut::show(STD, "dynamic_cast checks (compile time)", kDynamicCastWorks);

    tut::section(STD, "2. try/catch");
    tut::show(STD, "parse_digit('7') (compile time)", std::integral_constant<int, parse_digit('7')>::value);
    volatile char bad = 'x';
    tut::show(STD, "parse_digit('x') (runtime, caught)", parse_digit(bad));
    tut::show(STD, "g_parseFailures", g_parseFailures);

    tut::section(STD, "3. unions and uninitialised locals");
    tut::show(STD, "switch_member()", switch_member());
    tut::show(STD, "late_init(true)", late_init(true));

    tut::section(STD, "4. class-type template parameters");
    tut::show(STD, "GrandPiano::name", GrandPiano::name);
    tut::show(STD, "Strings::program", Strings::program);
#if L10_HAS_FLOAT_NTTP
    tut::show(STD, "Tone<440.0>::period_ms", A4::period_ms);
#else
    tut::note(STD, "floating-point template parameters not supported by this compiler");
#endif
}

} // namespace l10_cpp20_virtual_try_union

#endif // TUTORIAL_INCLUDE(20)
