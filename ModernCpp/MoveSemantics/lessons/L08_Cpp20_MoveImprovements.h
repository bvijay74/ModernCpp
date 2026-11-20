// =============================================================================
// L08 [C++20]  more implicit moves, pack init-capture, concepts
// =============================================================================
// C++20 changes for move semantics:
//   * MORE IMPLICIT MOVES (P1825): `return x;` and `throw x;` now also move
//     when x is an rvalue-reference parameter (compare L03-E3)
//   * pack expansion in init-capture: [...args = std::forward<Args>(args)]
//     moves a whole parameter pack into a lambda
//   * concepts: std::movable, std::copyable, std::constructible_from ... make
//     requirements on forwarded types readable, and errors short
//   * std::remove_cvref_t<T>: the type a forwarding reference refers to
//   * std::ranges::move(range, out) moves a whole range
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(20)

#include <algorithm>
#include <concepts>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l08_cpp20_move_improvements {

const int STD = 20;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. Implicit move of rvalue-reference parameters
// -----------------------------------------------------------------------------
// `t` is a named rvalue reference (an lvalue). Before C++20, `return t;`
// copied it. Since C++20 it is moved, like a local variable would be.
inline Tracked pass_back(Tracked&& t) { return t; }

inline std::unique_ptr<int> pass_through(std::unique_ptr<int>&& p) { return p; }   // L03-E3 compiles now

// ERROR DEMO [L08-E1]  fails in: C++20 only (valid from C++23 - see L09)
// Returning `t` as a REFERENCE (Tracked&&) still treats it as an lvalue in
// C++20. C++23 makes the returned name an xvalue, so this binds.
#if TUTORIAL_FORCE_ERRORS
Tracked&& give_back(Tracked&& t) { return t; }
#endif

// -----------------------------------------------------------------------------
// 2. Moving a whole pack into a lambda
// -----------------------------------------------------------------------------
#if defined(__cpp_init_captures) && __cpp_init_captures >= 201803L
#define L08_HAS_PACK_CAPTURE 1
template <typename... Ts>
auto make_chord(Ts&&... notes)
{
    // Each note is forwarded into the lambda: copied if it was an lvalue,
    // moved if it was an rvalue.
    return [... ns = std::forward<Ts>(notes)] {
        std::string names;
        ((names += ns.name() + " "), ...);
        return names;
    };
}
#else
#define L08_HAS_PACK_CAPTURE 0
#endif

// -----------------------------------------------------------------------------
// 3. Concepts for move semantics
// -----------------------------------------------------------------------------
struct NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;
};

static_assert(std::movable<Tracked> && std::copyable<Tracked>);
static_assert(std::movable<std::unique_ptr<int>> && !std::copyable<std::unique_ptr<int>>);
static_assert(!std::movable<NonMovable>);

// ERROR DEMO [L08-E2]  fails in: every standard
// unique_ptr is movable but not copyable.
#if TUTORIAL_FORCE_ERRORS
static_assert(std::copyable<std::unique_ptr<int>>);
#endif

// Accept anything movable, by forwarding reference. remove_cvref_t strips
// the `&` that T gets for lvalues, so we test the object type itself.
template <typename T>
    requires std::movable<std::remove_cvref_t<T>>
std::vector<std::remove_cvref_t<T>> one_element_vector(T&& item)
{
    std::vector<std::remove_cvref_t<T>> v;
    v.push_back(std::forward<T>(item));
    return v;
}

// ERROR DEMO [L08-E3]  fails in: every standard
// The constraint rejects a non-movable type with a short, readable message
// ("constraints not satisfied ... std::movable") instead of a long error from
// deep inside std::vector.
#if TUTORIAL_FORCE_ERRORS
void error_demo_constraint() { one_element_vector(NonMovable{}); }
#endif

// The classic forwarding-constructor trap (see L10), fixed with a constraint:
// only strings (or things convertible to one) may use the template.
class Musician {
public:
    template <typename S>
        requires std::constructible_from<std::string, S>
    explicit Musician(S&& name) : name_(std::forward<S>(name))
    {
    }
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L08", "implicit moves, pack init-capture, concepts");

    tut::section(STD, "1. returning an rvalue-reference parameter moves");
    Tracked::reset();
    Tracked back = pass_back(Tracked("param"));
    tut::show(STD, "pass_back(Tracked&& t) { return t; }", Tracked::summary());
    auto p = pass_through(std::make_unique<int>(7));
    tut::show(STD, "*pass_through(unique_ptr&&)", *p);

    tut::section(STD, "2. pack init-capture");
#if L08_HAS_PACK_CAPTURE
    Tracked c("C4"), e("E4");
    Tracked::reset();
    auto chord = make_chord(c, std::move(e), Tracked("G4"));
    tut::show(STD, "make_chord(lvalue, std::move, temporary)", Tracked::summary());
    tut::show(STD, "chord()", chord());
#else
    tut::note(STD, "not supported by this compiler (__cpp_init_captures >= 201803L)");
#endif

    tut::section(STD, "3. concepts");
    tut::show(STD, "std::movable<std::unique_ptr<int>>", std::movable<std::unique_ptr<int>>);
    tut::show(STD, "std::copyable<std::unique_ptr<int>>", std::copyable<std::unique_ptr<int>>);
    tut::show(STD, "std::movable<NonMovable>", std::movable<NonMovable>);
    Tracked solo("solo");
    Tracked::reset();
    auto v1 = one_element_vector(solo);
    tut::show(STD, "one_element_vector(lvalue)", Tracked::summary());
    Tracked::reset();
    auto v2 = one_element_vector(std::move(solo));
    tut::show(STD, "one_element_vector(std::move(x))", Tracked::summary());

    Musician m1("Chopin");
    Musician m2(m1);   // the copy constructor, not the template (constraint fails for Musician&)
    tut::show(STD, "Musician m2(m1).name()", m2.name());

    tut::section(STD, "4. std::ranges::move");
    std::vector<Tracked> from;
    from.reserve(2);
    from.emplace_back("x");
    from.emplace_back("y");
    std::vector<Tracked> to;
    to.reserve(2);
    Tracked::reset();
    std::ranges::move(from, std::back_inserter(to));
    tut::show(STD, "std::ranges::move(from, back_inserter)", Tracked::summary());
}

} // namespace l08_cpp20_move_improvements

#endif // TUTORIAL_INCLUDE(20)
