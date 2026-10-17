// =============================================================================
// L06 [C++20]  consteval: immediate functions
// =============================================================================
// constexpr function   MAY  run at compile time (and may also run at runtime)
// consteval function   MUST run at compile time - every call has to be a
//                      constant expression, or the program doesn't compile
//
// Rules for consteval ("immediate") functions:
//   * every call must produce a constant: the arguments must be constants
//   * the result can still be USED at runtime: `int r = square(4);` is fine,
//     because square(4) itself is evaluated by the compiler
//   * a consteval function can call constexpr functions, but a constexpr
//     function can't call a consteval one with its own (runtime) parameters
//   * you can't take its address and keep it for runtime
//
// Typical uses: values that must be checked at compile time (a constructor
// that rejects bad input), compile-time hashing, guaranteed lookup tables.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace l06_cpp20_consteval {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. consteval vs constexpr
// -----------------------------------------------------------------------------
constexpr int square_constexpr(int x) { return x * x; }   // may run at runtime
consteval int square(int x) { return x * x; }             // never runs at runtime

static_assert(square(9) == 81);
constexpr int kNine = square(3);

// ERROR DEMO [L06-E1]  fails in: every standard
// x is a runtime parameter, so square(x) can't be evaluated by the compiler.
// The same call with square_constexpr(x) would be fine.
#if TUTORIAL_FORCE_ERRORS
inline int runtime_square(int x) { return square(x); }
#endif

// ERROR DEMO [L06-E2]  fails in: every standard
// A constexpr function can't pass its own parameter to a consteval function:
// the constexpr function might be called at runtime. (C++23 relaxes this for
// templates and lambdas only - see L11.)
#if TUTORIAL_FORCE_ERRORS
constexpr int twice_square(int x) { return square(x) * 2; }
#endif

// ERROR DEMO [L06-E3]  fails in: every standard
// Taking the address of a consteval function and keeping it for runtime.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_address()
{
    auto fn = &square;
    (void)fn;
}
#endif

// -----------------------------------------------------------------------------
// 2. Compile-time validated values: a consteval constructor
// -----------------------------------------------------------------------------
// A MidiNote can only be created from a constant 0..127. Reaching the `throw`
// during constant evaluation is a compile error, so bad values never get in.
class MidiNote {
public:
    consteval MidiNote(int note) : note_(note)
    {
        if (note < 0 || note > 127)
            throw "MIDI note numbers are 0..127";
    }
    constexpr int value() const { return note_; }

private:
    int note_;
};

// An ordinary RUNTIME function taking a MidiNote. play(60) converts 60 to a
// MidiNote at compile time, so every call is checked by the compiler.
inline std::string play(MidiNote n) { return "playing MIDI note " + std::to_string(n.value()); }

constexpr MidiNote kMiddleC{60};
static_assert(kMiddleC.value() == 60);

// ERROR DEMO [L06-E4]  fails in: every standard
// 128 is out of range: the constructor reaches the throw.
#if TUTORIAL_FORCE_ERRORS
inline std::string error_demo_note() { return play(128); }
#endif

// -----------------------------------------------------------------------------
// 3. Compile-time string hashing: switch on a string
// -----------------------------------------------------------------------------
// FNV-1a hash. constexpr, so the SAME function can hash runtime input...
constexpr std::uint32_t fnv1a(std::string_view s)
{
    std::uint32_t hash = 2166136261u;
    for (char c : s) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 16777619u;
    }
    return hash;
}

// ...and a consteval user-defined literal for the case labels: "play"_hash is
// guaranteed to be computed by the compiler.
consteval std::uint32_t operator""_hash(const char* s, std::size_t n) { return fnv1a({s, n}); }

static_assert("play"_hash == fnv1a("play"));

inline std::string dispatch(std::string_view command)   // command is a runtime value
{
    switch (fnv1a(command)) {
    case "play"_hash:  return "start playback";
    case "stop"_hash:  return "stop playback";
    case "tempo"_hash: return "change tempo";
    default:           return "unknown command";
    }
}

// -----------------------------------------------------------------------------
// 4. consteval lambdas and calling constexpr from consteval
// -----------------------------------------------------------------------------
constexpr int sum_to(int n)
{
    int s = 0;
    for (int i = 1; i <= n; ++i)
        s += i;
    return s;
}

consteval int sum_to_checked(int n)   // consteval may call constexpr functions
{
    if (n < 0)
        throw "n must not be negative";
    return sum_to(n);
}
static_assert(sum_to_checked(100) == 5050);

constexpr auto kCube = [](int x) consteval { return x * x * x; };
static_assert(kCube(4) == 64);

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L06", "consteval");

    tut::section(STD, "1. consteval vs constexpr");
    volatile int n = 7;
    tut::show(STD, "square_constexpr(runtime 7)", square_constexpr(n));
    int fromConstant = square(4);   // evaluated by the compiler, used at runtime
    tut::show(STD, "int r = square(4)", fromConstant);
    tut::show(STD, "kNine = square(3)", kNine);

    tut::section(STD, "2. consteval constructor");
    tut::show(STD, "play(60)", play(60));
    tut::show(STD, "play(kMiddleC)", play(kMiddleC));
    tut::note(STD, "play(128) does not compile - see ERROR DEMO [L06-E4]");

    tut::section(STD, "3. switch on a string via consteval hash");
    const char* commands[] = {"play", "tempo", "rewind"};
    for (const char* c : commands)
        tut::show(STD, std::string("dispatch(\"") + c + "\")", dispatch(c));
    tut::show(STD, "\"play\"_hash", "play"_hash);

    tut::section(STD, "4. consteval calling constexpr, consteval lambda");
    tut::show(STD, "sum_to_checked(100)", sum_to_checked(100));
    tut::show(STD, "kCube(4)", kCube(4));
}

} // namespace l06_cpp20_consteval

#endif // TUTORIAL_INCLUDE(20)
