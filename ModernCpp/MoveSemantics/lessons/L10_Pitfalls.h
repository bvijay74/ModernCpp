// =============================================================================
// L10 [C++11]  pitfalls
// =============================================================================
// 1. Using an object after moving from it
// 2. Forwarding (or moving) the same argument twice
// 3. A forwarding constructor that hijacks the copy constructor
// 4. `return std::move(local);` makes things worse, not better
// 5. Moving through a const reference silently copies
// 6. Dangling references: std::move doesn't extend lifetimes
// (Tagged C++11 because every pitfall exists from C++11 on.)
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(11)

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l10_pitfalls {

const int STD = 11;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 2. Forwarding the same argument twice
// -----------------------------------------------------------------------------
template <typename T>
void store_twice_bad(std::vector<Tracked>& v, T&& item)
{
    v.push_back(std::forward<T>(item));   // may move from item...
    v.push_back(std::forward<T>(item));   // ...so this may push a moved-from object
}

template <typename T>
void store_twice_good(std::vector<Tracked>& v, T&& item)
{
    v.push_back(item);                    // copy for every use but the last
    v.push_back(std::forward<T>(item));   // forward only on the LAST use
}

// -----------------------------------------------------------------------------
// 3. The forwarding constructor that hijacks copying
// -----------------------------------------------------------------------------
// For a NON-CONST lvalue Singer, the template with S = Singer& is a better
// match than the copy constructor (which needs const Singer&). The template
// then tries to make a std::string from a Singer.
class Singer {
public:
    template <typename S>
    explicit Singer(S&& name) : name_(std::forward<S>(name))
    {
    }
    Singer(const Singer&) = default;
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// ERROR DEMO [L10-E1]  fails in: every standard
// Copying a non-const Singer picks the template, not the copy constructor.
#if TUTORIAL_FORCE_ERRORS
std::string error_demo_hijack()
{
    Singer a("Callas");
    Singer b(a);
    return b.name();
}
#endif

// The fix (C++11): disable the template when S is Singer itself.
// (C++20 can say the same with a requires-clause - see L08's Musician.)
class FixedSinger {
public:
    template <typename S,
              typename = typename std::enable_if<
                  !std::is_same<typename std::decay<S>::type, FixedSinger>::value>::type>
    explicit FixedSinger(S&& name) : name_(std::forward<S>(name))
    {
    }
    FixedSinger(const FixedSinger&) = default;
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

// -----------------------------------------------------------------------------
// 4. return std::move(local) is a pessimisation
// -----------------------------------------------------------------------------
inline Tracked make_good()
{
    Tracked t("good");
    return t;   // NRVO, or at worst an implicit move
}

// std::move turns `t` into an expression that is no longer just the name of
// a local, so the compiler is not allowed to elide: you always pay a move.
// GCC and Clang warn about this (-Wpessimizing-move); silenced here so the
// tutorial builds without warnings.
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 9)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpessimizing-move"
#endif
inline Tracked make_pessimised()
{
    Tracked t("pessimised");
    return std::move(t);
}
#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 9)
#pragma GCC diagnostic pop
#endif

// -----------------------------------------------------------------------------
// 6. Dangling references
// -----------------------------------------------------------------------------
// Lifetime extension (L01) only works when a reference binds DIRECTLY to a
// temporary. Through std::move it doesn't:
//     const std::string& ok  = std::string("x");              // extended
//     std::string&&      bad = std::move(std::string("x"));   // DANGLING
// The temporary dies at the semicolon; `bad` refers to a destroyed object.
// Compilers can't always warn - there is no demo here because it compiles
// (and then misbehaves at runtime).

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L10", "pitfalls");

    tut::section(STD, "1. use after move");
    Tracked sample("sample");
    Tracked owner(std::move(sample));
    tut::show(STD, "sample.name() after the move", sample.name());
    tut::note(STD, "std types are 'valid but unspecified' after a move: assign before reuse");
    std::string text = "constexpr";
    std::string taken = std::move(text);
    text = "assigned again";   // always fine
    tut::show(STD, "std::string reused after assignment", text);

    tut::section(STD, "2. forwarding the same argument twice");
    std::vector<Tracked> bad;
    bad.reserve(2);
    store_twice_bad(bad, Tracked("chord"));
    tut::show(STD, "store_twice_bad: [0], [1]", bad[0].name() + ", " + bad[1].name());
    std::vector<Tracked> good;
    good.reserve(2);
    store_twice_good(good, Tracked("chord"));
    tut::show(STD, "store_twice_good: [0], [1]", good[0].name() + ", " + good[1].name());

    tut::section(STD, "3. forwarding constructor vs copy constructor");
    FixedSinger diva("Callas");
    FixedSinger copy(diva);   // the copy constructor, as intended
    tut::show(STD, "FixedSinger copy(diva).name()", copy.name());

    tut::section(STD, "4. return t;  vs  return std::move(t);");
    Tracked::reset();
    Tracked g = make_good();
    tut::show(STD, "return t;", Tracked::summary());
    Tracked::reset();
    Tracked p = make_pessimised();
    tut::show(STD, "return std::move(t);", Tracked::summary());

    tut::section(STD, "5. moving through const& copies");
    std::vector<Tracked> src;
    src.reserve(3);
    src.emplace_back("a");
    src.emplace_back("b");
    src.emplace_back("c");
    std::vector<Tracked> dst;
    dst.reserve(3);
    Tracked::reset();
    for (const Tracked& t : src)
        dst.push_back(std::move(t));   // const Tracked&& -> copy constructor
    tut::show(STD, "for (const auto& t : v) push_back(std::move(t))", Tracked::summary());
    dst.clear();
    Tracked::reset();
    for (Tracked& t : src)
        dst.push_back(std::move(t));
    tut::show(STD, "for (auto& t : v) push_back(std::move(t))", Tracked::summary());
}

} // namespace l10_pitfalls

#endif // TUTORIAL_INCLUDE(11)
