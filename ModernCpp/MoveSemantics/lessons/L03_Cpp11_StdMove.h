// =============================================================================
// L03 [C++11]  std::move: what it really does
// =============================================================================
// std::move doesn't move anything. It is a CAST to an rvalue reference:
//     static_cast<typename std::remove_reference<T>::type&&>(t)
// It says "I'm done with this object; you may steal from it". The actual
// moving happens later, if a move constructor or move assignment is chosen.
//
// Consequences:
//   * std::move on a const object gives `const T&&`, which the move
//     constructor (T&&) can't accept - so the COPY constructor runs, silently.
//   * after a move, the object is in a "valid but unspecified" state: you may
//     assign to it or destroy it, but don't rely on its value.
//   * std::vector only moves its elements when it grows if the move
//     constructor is noexcept; otherwise it copies, to keep its strong
//     exception guarantee (std::move_if_noexcept).
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

namespace l03_cpp11_std_move {

const int STD = 11;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. std::move is a cast: our own version
// -----------------------------------------------------------------------------
template <typename T>
typename std::remove_reference<T>::type&& my_move(T&& t) noexcept
{
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

static_assert(std::is_same<decltype(my_move(std::declval<std::string&>())), std::string&&>::value,
              "my_move(lvalue) yields an rvalue reference");

// -----------------------------------------------------------------------------
// 2. Sink parameters: take by value, then move into place
// -----------------------------------------------------------------------------
// One constructor serves every caller well:
//   lvalue argument           -> 1 copy (into the parameter) + 1 move (into the member)
//   temporary argument        -> 0 copies, 1 move (the parameter is built in place)
//   std::move(lvalue) argument -> 0 copies, 2 moves
class Voice {
public:
    explicit Voice(Tracked sample) : sample_(std::move(sample)) {}
    const Tracked& sample() const { return sample_; }

private:
    Tracked sample_;
};

// -----------------------------------------------------------------------------
// 3. std::move on a const object copies
// -----------------------------------------------------------------------------
inline void consume(Tracked&& t) { Tracked taken(std::move(t)); }

// ERROR DEMO [L03-E1]  fails in: every standard
// std::move(c) is `const Tracked&&`, which can't bind to `Tracked&&`.
// (With a `Tracked` or `const Tracked&` parameter it compiles - and copies.)
#if TUTORIAL_FORCE_ERRORS
void error_demo_move_const()
{
    const Tracked c("const");
    consume(std::move(c));
}
#endif

// -----------------------------------------------------------------------------
// 4. noexcept and std::vector growth
// -----------------------------------------------------------------------------
// The same as Tracked, but its move constructor is NOT noexcept.
struct MaybeThrowingMove {
    Tracked t;
    explicit MaybeThrowingMove(const char* name) : t(name) {}
    MaybeThrowingMove(const MaybeThrowingMove&) = default;
    MaybeThrowingMove(MaybeThrowingMove&& other) : t(std::move(other.t)) {}   // no noexcept
};

static_assert(std::is_nothrow_move_constructible<Tracked>::value, "Tracked moves can't throw");

// ERROR DEMO [L03-E2]  fails in: every standard
// MaybeThrowingMove's move constructor isn't noexcept, so the check fails -
// and std::vector will copy instead of move when it reallocates.
#if TUTORIAL_FORCE_ERRORS
static_assert(std::is_nothrow_move_constructible<MaybeThrowingMove>::value,
              "vector<MaybeThrowingMove> will copy when it grows");
#endif

template <typename T>
std::string fill_vector(int count)
{
    std::vector<T> v;   // no reserve(): let it reallocate a few times
    Tracked::reset();
    for (int i = 0; i < count; ++i)
        v.emplace_back("x");
    return Tracked::summary();
}

// -----------------------------------------------------------------------------
// 5. Returning a parameter: copy or move?
// -----------------------------------------------------------------------------
// A local variable or by-value parameter in a return statement is moved
// automatically. An rvalue-REFERENCE parameter was not, until C++20.

// ERROR DEMO [L03-E3]  fails in: C++11, C++14, C++17 (valid from C++20 - see L08)
// p is a named rvalue reference (an lvalue), so before C++20 `return p;`
// tries to COPY the move-only unique_ptr. C++20 moves it implicitly.
// (Clang applies the C++20 rule in older modes too, so it accepts this.)
#if TUTORIAL_FORCE_ERRORS
std::unique_ptr<int> pass_through(std::unique_ptr<int>&& p) { return p; }
#endif

inline std::unique_ptr<int> pass_through_fixed(std::unique_ptr<int>&& p) { return std::move(p); }

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L03", "std::move");

    tut::section(STD, "1. std::move is only a cast");
    Tracked a("a");
    Tracked::reset();
    my_move(a);                       // a cast on its own does nothing
    tut::show(STD, "my_move(a); on its own", Tracked::summary());
    tut::show(STD, "a.name() is unchanged", a.name());
    Tracked b(my_move(a));            // the move CONSTRUCTOR does the work
    tut::show(STD, "Tracked b(my_move(a))", Tracked::summary());

    tut::section(STD, "2. sink parameter: take by value, then move");
    Tracked piano("piano");
    Tracked::reset();
    Voice v1(piano);
    tut::show(STD, "Voice(lvalue)", Tracked::summary());
    Tracked::reset();
    Voice v2(Tracked("organ"));
    tut::show(STD, "Voice(temporary)", Tracked::summary());
    Tracked::reset();
    Voice v3(std::move(piano));
    tut::show(STD, "Voice(std::move(lvalue))", Tracked::summary());

    tut::section(STD, "3. the moved-from object");
    tut::show(STD, "piano.name() after the move", piano.name());
    piano = Tracked("new piano");   // assigning a new value is always fine
    tut::show(STD, "after piano = Tracked(\"new piano\")", piano.name());

    tut::section(STD, "4. std::move on a const object copies");
    const Tracked c("const sample");
    Tracked::reset();
    Tracked fromConst(std::move(c));
    tut::show(STD, "Tracked t(std::move(const))", Tracked::summary());

    tut::section(STD, "5. noexcept decides: move or copy on vector growth");
    tut::show(STD, "vector<Tracked>, 17 x emplace_back", fill_vector<Tracked>(17));
    tut::show(STD, "vector<MaybeThrowingMove>, 17 x ...", fill_vector<MaybeThrowingMove>(17));
    tut::note(STD, "the exact numbers depend on the library's growth factor");

    tut::section(STD, "6. returning an rvalue-reference parameter");
    std::unique_ptr<int> p(new int(42));
    std::unique_ptr<int> q = pass_through_fixed(std::move(p));
    tut::show(STD, "*pass_through_fixed(std::move(p))", *q);
    tut::show(STD, "p is now empty", p == nullptr);
}

} // namespace l03_cpp11_std_move

#endif // TUTORIAL_INCLUDE(11)
