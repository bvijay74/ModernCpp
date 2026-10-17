// =============================================================================
// L12 [C++23]  relaxed rules and more constexpr library
// =============================================================================
// Language changes in C++23:
//   * static constexpr variables inside constexpr functions (P2647)
//       a lookup table can now live inside the function that uses it
//   * non-literal variables, static/thread_local variables, goto and labels
//     may APPEAR in a constexpr function, as long as constant evaluation never
//     reaches them (P2242)
//   * a constexpr function no longer has to be usable in a constant
//     expression for SOME argument (P2448). Before, "constexpr" on a function
//     that could never be constant was an error the compiler wasn't required
//     to diagnose. Now it's allowed, which matters for generic code.
// Library:
//   * std::unique_ptr is constexpr (P2273)
//   * std::bitset is constexpr (P2417)
//   * std::to_chars / from_chars for integers are constexpr (P2291)
//   * std::optional and std::variant are fully constexpr (P2231)
//   * some <cmath> functions are constexpr (P0533) - few compilers so far
//
// Each part is checked with its feature-test macro, so the lesson builds on
// every compiler and tells you what yours is missing.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(23)

#include <bitset>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

namespace l12_cpp23_relaxed_rules {

const int STD = 23;

// -----------------------------------------------------------------------------
// 1. static constexpr locals (compare L08-E2, which fails in C++20)
// -----------------------------------------------------------------------------
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L
#define L12_HAS_STATIC_CONSTEXPR_LOCAL 1
constexpr int semitone_offset(int degree)
{
    static constexpr int kMajorScale[] = {0, 2, 4, 5, 7, 9, 11};   // C++23: OK
    return kMajorScale[degree % 7] + 12 * (degree / 7);
}
static_assert(semitone_offset(4) == 7);    // the 5th degree: G above C
static_assert(semitone_offset(7) == 12);   // one octave up
#else
#define L12_HAS_STATIC_CONSTEXPR_LOCAL 0
#endif

// -----------------------------------------------------------------------------
// 2. Code that constant evaluation never reaches (compare L03-E7)
// -----------------------------------------------------------------------------
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202110L
#define L12_HAS_UNREACHED_STATICS 1
constexpr int lookup_or_log(int key)
{
    if (key >= 0 && key < 4)
        return key * 10;    // the only path used at compile time
    static int misses = 0;  // C++23: allowed, because compile time never gets here
    ++misses;
    goto done;              // goto too
done:
    return -misses;
}
static_assert(lookup_or_log(3) == 30);

// ERROR DEMO [L12-E1]  fails in: every standard
// The rule is "never REACHED". Calling it with a key that hits the static
// variable during constant evaluation is still an error.
#if TUTORIAL_FORCE_ERRORS
static_assert(lookup_or_log(9) == -1);
#endif
#else
#define L12_HAS_UNREACHED_STATICS 0
#endif

// -----------------------------------------------------------------------------
// 3. constexpr std::unique_ptr
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_constexpr_memory) && __cpp_lib_constexpr_memory >= 202202L
#define L12_HAS_CONSTEXPR_UNIQUE_PTR 1
struct Voice {
    int note;
    int velocity;
};

constexpr int loudest_of(int a, int b)
{
    auto first = std::make_unique<Voice>(Voice{60, a});    // freed automatically
    auto second = std::make_unique<Voice>(Voice{64, b});
    return first->velocity > second->velocity ? first->velocity : second->velocity;
}
static_assert(loudest_of(80, 100) == 100);

// ERROR DEMO [L12-E2]  fails in: every standard
// Same rule as std::vector in L09: a constexpr unique_ptr VARIABLE would keep
// compile-time heap memory alive at runtime.
#if TUTORIAL_FORCE_ERRORS
constexpr auto kLeak = std::make_unique<int>(42);
#endif
#else
#define L12_HAS_CONSTEXPR_UNIQUE_PTR 0
#endif

// -----------------------------------------------------------------------------
// 4. constexpr std::bitset
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_constexpr_bitset) && __cpp_lib_constexpr_bitset >= 202202L
#define L12_HAS_CONSTEXPR_BITSET 1
// Which of the 12 pitch classes are in a C major chord (C, E, G)?
constexpr std::bitset<12> chord_mask(int root)
{
    std::bitset<12> mask;
    mask.set(root % 12);         // root
    mask.set((root + 4) % 12);   // major third
    mask.set((root + 7) % 12);   // fifth
    return mask;
}
constexpr std::bitset<12> kCMajor = chord_mask(0);
static_assert(kCMajor.count() == 3 && kCMajor.test(4) && !kCMajor.test(5));
static_assert((chord_mask(0) & chord_mask(7)).count() == 1);   // C and G major share only G
#else
#define L12_HAS_CONSTEXPR_BITSET 0
#endif

// -----------------------------------------------------------------------------
// 5. constexpr std::to_chars (integers)
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_constexpr_charconv) && __cpp_lib_constexpr_charconv >= 202207L
#define L12_HAS_CONSTEXPR_CHARCONV 1
struct SmallText {
    char text[8]{};
};
constexpr SmallText midi_to_text(int midi)   // compare L09: std::to_string isn't constexpr
{
    SmallText out;
    std::to_chars(out.text, out.text + 7, midi);
    return out;
}
constexpr SmallText kMidiText = midi_to_text(108);
static_assert(kMidiText.text[0] == '1' && kMidiText.text[2] == '8');
#else
#define L12_HAS_CONSTEXPR_CHARCONV 0
#endif

// -----------------------------------------------------------------------------
// 6. constexpr std::optional
// -----------------------------------------------------------------------------
constexpr std::optional<int> find_note(const char* name)
{
    constexpr const char* kNames[] = {"C", "D", "E", "F", "G", "A", "B"};
    constexpr int kOffsets[] = {0, 2, 4, 5, 7, 9, 11};
    for (int i = 0; i < 7; ++i)
        if (kNames[i][0] == name[0] && name[1] == '\0')
            return kOffsets[i];
    return std::nullopt;
}
static_assert(find_note("A").value() == 9);
static_assert(!find_note("H").has_value());

// -----------------------------------------------------------------------------
// 7. constexpr <cmath> (rarely implemented yet)
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_constexpr_cmath) && __cpp_lib_constexpr_cmath >= 202202L
#define L12_HAS_CONSTEXPR_CMATH 1
static_assert(std::fabs(-2.5) == 2.5);
static_assert(std::fmax(1.0, 3.0) == 3.0);
#else
#define L12_HAS_CONSTEXPR_CMATH 0
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L12", "relaxed rules, constexpr library");
    const char* missing = "not supported by this compiler / standard library";

    tut::section(STD, "1. static constexpr local table");
#if L12_HAS_STATIC_CONSTEXPR_LOCAL
    tut::show(STD, "semitone_offset(4)", semitone_offset(4));
    tut::show(STD, "semitone_offset(7)", semitone_offset(7));
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "2. statics/goto on paths not taken");
#if L12_HAS_UNREACHED_STATICS
    tut::show(STD, "lookup_or_log(3) (compile time)", std::integral_constant<int, lookup_or_log(3)>::value);
    volatile int key = 9;
    tut::show(STD, "lookup_or_log(9) (runtime)", lookup_or_log(key));
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "3. std::unique_ptr");
#if L12_HAS_CONSTEXPR_UNIQUE_PTR
    tut::show(STD, "loudest_of(80, 100)", std::integral_constant<int, loudest_of(80, 100)>::value);
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "4. std::bitset");
#if L12_HAS_CONSTEXPR_BITSET
    tut::show(STD, "chord_mask(0) (B..C)", kCMajor.to_string());
    tut::show(STD, "kCMajor.count()", kCMajor.count());
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "5. std::to_chars");
#if L12_HAS_CONSTEXPR_CHARCONV
    tut::show(STD, "midi_to_text(108)", kMidiText.text);
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "6. std::optional");
    tut::show(STD, "find_note(\"A\").value()", find_note("A").value());
    tut::show(STD, "find_note(\"H\").has_value()", find_note("H").has_value());

    tut::section(STD, "7. <cmath>");
#if L12_HAS_CONSTEXPR_CMATH
    tut::show(STD, "std::fabs(-2.5) (compile time)", std::fabs(-2.5));
#else
    tut::note(STD, missing);
#endif
}

} // namespace l12_cpp23_relaxed_rules

#endif // TUTORIAL_INCLUDE(23)
