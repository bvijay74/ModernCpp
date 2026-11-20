// =============================================================================
// L04 [C++11]  perfect forwarding
// =============================================================================
// The problem: a wrapper function takes an argument and passes it on. Inside
// the wrapper the parameter has a NAME, so it is an lvalue - and an rvalue
// that came in is passed on as an lvalue (copied instead of moved).
//
// The solution has three parts:
//   1. FORWARDING REFERENCE: `T&&` where T is a deduced template parameter
//      (or `auto&&`). It binds to everything, and T remembers what it was:
//          called with an lvalue of type X  ->  T = X&
//          called with an rvalue of type X  ->  T = X
//   2. REFERENCE COLLAPSING makes T&& work out:
//          X& &&  -> X&      X&& &&  -> X&&      (& wins)
//   3. std::forward<T>(x): a conditional std::move - it casts back to an
//      rvalue only if T says the argument was an rvalue.
//
// Not a forwarding reference: `std::vector<T>&&`, `const T&&`, or `X&&` for a
// concrete type X. Those are plain rvalue references.
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(11)

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l04_cpp11_perfect_forwarding {

const int STD = 11;
using tut::Tracked;

inline void store(const Tracked& t) { Tracked kept(t); }    // copies
inline void store(Tracked&& t) { Tracked kept(std::move(t)); }   // moves

// -----------------------------------------------------------------------------
// 1. The problem: forwarding without std::forward
// -----------------------------------------------------------------------------
template <typename T>
void wrapper_bad(T&& arg)
{
    store(arg);   // `arg` is an lvalue here -> always the copying overload
}

template <typename T>
void wrapper_good(T&& arg)
{
    store(std::forward<T>(arg));   // rvalue in -> rvalue out
}

// -----------------------------------------------------------------------------
// 2. What does T become? (reference collapsing)
// -----------------------------------------------------------------------------
template <typename T>
const char* deduced(T&&)
{
    return std::is_lvalue_reference<T>::value
               ? (std::is_const<typename std::remove_reference<T>::type>::value ? "T = const X&  -> param const X&"
                                                                                : "T = X&        -> param X&")
               : "T = X         -> param X&&";
}

// -----------------------------------------------------------------------------
// 3. std::forward, written out
// -----------------------------------------------------------------------------
template <typename T>
T&& my_forward(typename std::remove_reference<T>::type& t) noexcept
{
    return static_cast<T&&>(t);   // T = X& -> X& (collapsing); T = X -> X&&
}

// ERROR DEMO [L04-E1]  fails in: every standard
// std::forward needs the template argument: it can't be deduced, because the
// parameter type is remove_reference<T>::type& (a "non-deduced context").
#if TUTORIAL_FORCE_ERRORS
void error_demo_forward(Tracked&& arg)
{
    store(std::forward(arg));
}
#endif

// -----------------------------------------------------------------------------
// 4. Variadic forwarding: a factory (std::make_unique arrived in C++14)
// -----------------------------------------------------------------------------
struct Instrument {
    Tracked sample;
    std::string name;
    int midiChannel;
    Instrument(Tracked s, std::string n, int ch) : sample(std::move(s)), name(std::move(n)), midiChannel(ch) {}
};

template <typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// -----------------------------------------------------------------------------
// 5. Not every T&& is a forwarding reference
// -----------------------------------------------------------------------------
template <typename T>
std::size_t count_rvalue_vector(std::vector<T>&& v)   // rvalue reference to vector<T>
{
    return v.size();
}

// ERROR DEMO [L04-E2]  fails in: every standard
// std::vector<T>&& is NOT a forwarding reference (T&& must be the whole
// type), so an lvalue vector can't be passed.
#if TUTORIAL_FORCE_ERRORS
std::size_t error_demo_vector_rvalue()
{
    std::vector<int> notes(3);
    return count_rvalue_vector(notes);
}
#endif

template <typename T>
void const_rvalue_only(const T&&)
{
}

// ERROR DEMO [L04-E3]  fails in: every standard
// const T&& is not a forwarding reference either: it only binds to rvalues.
#if TUTORIAL_FORCE_ERRORS
void error_demo_const_rvalue()
{
    int x = 1;
    const_rvalue_only(x);
}
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L04", "perfect forwarding");

    tut::section(STD, "1. wrapper without / with std::forward");
    Tracked a("a");
    Tracked::reset();
    wrapper_bad(Tracked("temp"));
    tut::show(STD, "wrapper_bad(temporary)", Tracked::summary());
    Tracked::reset();
    wrapper_good(Tracked("temp"));
    tut::show(STD, "wrapper_good(temporary)", Tracked::summary());
    Tracked::reset();
    wrapper_good(a);
    tut::show(STD, "wrapper_good(lvalue)", Tracked::summary());

    tut::section(STD, "2. what T deduces to");
    int x = 1;
    const int cx = 2;
    tut::show(STD, "deduced(x)", deduced(x));
    tut::show(STD, "deduced(cx)", deduced(cx));
    tut::show(STD, "deduced(42)", deduced(42));
    tut::show(STD, "deduced(std::move(x))", deduced(std::move(x)));

    tut::section(STD, "3. my_forward<T> behaves like std::forward");
    Tracked b("b");
    Tracked::reset();
    store(my_forward<Tracked&>(b));
    tut::show(STD, "store(my_forward<Tracked&>(b))", Tracked::summary());
    Tracked::reset();
    store(my_forward<Tracked>(b));
    tut::show(STD, "store(my_forward<Tracked>(b))", Tracked::summary());

    tut::section(STD, "4. variadic factory make<T>(args...)");
    Tracked sample("piano-sample");
    Tracked::reset();
    std::unique_ptr<Instrument> p1 = make<Instrument>(sample, "Piano", 1);
    tut::show(STD, "make<Instrument>(lvalue, ...)", Tracked::summary());
    Tracked::reset();
    std::unique_ptr<Instrument> p2 = make<Instrument>(Tracked("organ-sample"), "Organ", 2);
    tut::show(STD, "make<Instrument>(temporary, ...)", Tracked::summary());
    tut::show(STD, "p2->name", p2->name);

    tut::section(STD, "5. push_back vs emplace_back");
    std::vector<Tracked> v;
    v.reserve(4);   // no reallocation noise
    Tracked::reset();
    v.push_back(Tracked("pushed"));    // build a temporary, then move it in
    tut::show(STD, "push_back(Tracked(\"pushed\"))", Tracked::summary());
    Tracked::reset();
    v.emplace_back("emplaced");        // forwards "emplaced" to the constructor
    tut::show(STD, "emplace_back(\"emplaced\")", Tracked::summary());

    tut::section(STD, "6. rvalue vector is not a forwarding reference");
    tut::show(STD, "count_rvalue_vector(std::vector<int>(5))", count_rvalue_vector(std::vector<int>(5)));
}

} // namespace l04_cpp11_perfect_forwarding

#endif // TUTORIAL_INCLUDE(11)
