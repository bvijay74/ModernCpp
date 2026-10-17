// =============================================================================
// L07 [C++20]  constinit: compile-time initialisation, runtime mutation
// =============================================================================
// Three keywords, three promises:
//   constexpr  value known at compile time AND the variable is const
//   constinit  INITIALISED at compile time, but the variable is NOT const -
//              you can change it at runtime
//   consteval  (functions only) must be evaluated at compile time
//
// Why constinit? Globals and statics in different .cpp files are initialised
// in an unspecified order when their initialiser runs at runtime ("dynamic
// initialisation"). If one global's initialiser reads another global that
// hasn't been initialised yet, you get the "static initialisation order
// fiasco". constinit guarantees the value is baked into the program image
// (static initialisation), so it is ready before ANY code runs - and the
// compiler refuses to build if that's not possible.
//
// constinit can only be used on variables with static or thread storage
// duration: globals, namespace-scope, static members, static/thread_local locals.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <cstdlib>

namespace l07_cpp20_constinit {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. constinit global: compile-time value, changeable at runtime
// -----------------------------------------------------------------------------
constexpr int default_tempo() { return 120; }

constinit int g_tempo = default_tempo();        // initialised by the compiler
constinit const char* g_instrument = "piano";   // pointer to a string literal is a constant

// ERROR DEMO [L07-E1]  fails in: every standard
// std::rand() runs at runtime, so this can't be static initialisation.
#if TUTORIAL_FORCE_ERRORS
constinit int g_random = std::rand();
#endif

// ERROR DEMO [L07-E2]  fails in: every standard
// constinit does NOT make the variable const, so its value can't be used in a
// constant expression - someone may have changed it.
#if TUTORIAL_FORCE_ERRORS
static_assert(g_tempo == 120);
#endif

// ERROR DEMO [L07-E3]  fails in: every standard
// constexpr already implies compile-time initialisation; they can't be combined.
#if TUTORIAL_FORCE_ERRORS
constexpr constinit int g_both = 1;
#endif

// -----------------------------------------------------------------------------
// 2. constinit objects of class type
// -----------------------------------------------------------------------------
// Any type with a constexpr constructor works - even one with mutating members.
class NoteCounter {
public:
    constexpr NoteCounter() = default;
    void add() { ++count_; }   // runtime-only member function is fine
    int count() const { return count_; }

private:
    int count_ = 0;
};

constinit NoteCounter g_noteCounter;   // ready before main(); no constructor runs at startup

// A constinit thread_local: each thread gets its own copy, and because the
// initialiser is a constant the compiler doesn't need a "first use" guard.
constinit thread_local int t_eventsThisThread = 0;

// -----------------------------------------------------------------------------
// 3. constinit static locals
// -----------------------------------------------------------------------------
inline int next_id()
{
    static constinit int counter = 1000;   // initialised at compile time, no init guard
    return counter++;
}

// ERROR DEMO [L07-E4]  fails in: every standard
// An ordinary local variable has automatic storage duration: not allowed.
#if TUTORIAL_FORCE_ERRORS
inline int error_demo_local()
{
    constinit int x = 1;
    return x;
}
#endif

// -----------------------------------------------------------------------------
// 4. A common idiom: extern constinit in a header
// -----------------------------------------------------------------------------
// In a real project, Settings.h would say
//     extern constinit int g_bufferSize;
// and Settings.cpp would define
//     constinit int g_bufferSize = 512;
// Every other .cpp can then read g_bufferSize safely during ITS static
// initialisation, because the value is never set by runtime code.

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L07", "constinit");

    tut::section(STD, "1. constinit global");
    tut::show(STD, "g_tempo at program start", g_tempo);
    g_tempo = 96;   // allowed: constinit is not const
    tut::show(STD, "g_tempo after g_tempo = 96", g_tempo);
    tut::show(STD, "g_instrument", g_instrument);

    tut::section(STD, "2. constinit class object / thread_local");
    g_noteCounter.add();
    g_noteCounter.add();
    tut::show(STD, "g_noteCounter.count() after 2 add()", g_noteCounter.count());
    ++t_eventsThisThread;
    tut::show(STD, "t_eventsThisThread", t_eventsThisThread);

    tut::section(STD, "3. static constinit local");
    tut::show(STD, "next_id()", next_id());
    tut::show(STD, "next_id()", next_id());

    g_tempo = default_tempo();   // reset, so running the lesson twice prints the same
}

} // namespace l07_cpp20_constinit

#endif // TUTORIAL_INCLUDE(20)
