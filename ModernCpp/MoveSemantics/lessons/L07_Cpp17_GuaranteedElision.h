// =============================================================================
// L07 [C++17]  guaranteed copy elision, node handles, try_emplace
// =============================================================================
// GUARANTEED COPY ELISION: a prvalue (a temporary) is no longer an object that
// gets copied or moved somewhere - it is a recipe that initialises its final
// destination directly. So
//     T x = T(args);          T make() { return T(args); }      T x = make();
// never call a copy or move constructor, and T doesn't even need one.
//   - returning a NAMED local (`T t; ...; return t;`) is different: that's
//     NRVO, still optional. Compilers usually do it, but T must be movable.
//
// Also new in C++17:
//   * map/set NODE HANDLES: extract() unlinks a node and insert() relinks it
//     in another container - the element itself is never copied or moved
//   * try_emplace / insert_or_assign: don't steal from your arguments when the
//     key already exists
//   * fold expressions make forwarding a whole pack easy
//   * std::optional and std::variant hold move-only types
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(17)

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace l07_cpp17_guaranteed_elision {

const int STD = 17;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. Guaranteed elision: no copy, no move, not even a move constructor needed
// -----------------------------------------------------------------------------
// A type that can be neither copied nor moved (it contains a std::mutex).
class AudioDevice {
public:
    explicit AudioDevice(std::string name) : name_(std::move(name)) {}
    AudioDevice(const AudioDevice&) = delete;
    AudioDevice(AudioDevice&&) = delete;
    const std::string& name() const { return name_; }

private:
    std::string name_;
    std::mutex lock_;
};

// C++17: returning a prvalue of a non-movable type is fine (compare L02-E2).
inline AudioDevice open_device(const std::string& name) { return AudioDevice(name); }

// ERROR DEMO [L07-E1]  fails in: every standard
// Returning a NAMED local needs the move constructor (NRVO isn't guaranteed).
#if TUTORIAL_FORCE_ERRORS
AudioDevice error_demo_named_return()
{
    AudioDevice d("named");
    return d;
}
#endif

// ERROR DEMO [L07-E2]  fails in: every standard
// Elision only applies to initialisation from a prvalue. Moving an existing
// object still needs the (deleted) move constructor.
#if TUTORIAL_FORCE_ERRORS
void error_demo_move_device()
{
    AudioDevice a = open_device("a");
    AudioDevice b = std::move(a);
}
#endif

inline Tracked make_prvalue() { return Tracked("prvalue"); }   // guaranteed: no copy/move

inline Tracked make_named()
{
    Tracked t("named");
    return t;   // NRVO: allowed, not required. Otherwise: 1 move.
}

// -----------------------------------------------------------------------------
// 2. Node handles: move elements between maps without touching them
// -----------------------------------------------------------------------------
using PatchMap = std::map<std::string, Tracked>;

// -----------------------------------------------------------------------------
// 3. Fold expressions + forwarding
// -----------------------------------------------------------------------------
template <typename... Ts>
void push_all(std::vector<Tracked>& v, Ts&&... items)
{
    (v.push_back(std::forward<Ts>(items)), ...);   // one push_back per argument
}

// An init-capture of a whole PACK - [...args = std::move(args)] - is C++20 (L08).

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L07", "guaranteed elision, node handles, try_emplace");

    tut::section(STD, "1. guaranteed copy elision");
    AudioDevice device = open_device("ASIO Output");   // non-movable, still returnable
    tut::show(STD, "open_device() of a non-movable type", device.name());
    Tracked::reset();
    Tracked p = make_prvalue();
    tut::show(STD, "Tracked p = make_prvalue()", Tracked::summary());
    Tracked::reset();
    Tracked q = Tracked(Tracked(Tracked("nested")));
    tut::show(STD, "Tracked(Tracked(Tracked(...)))", Tracked::summary());
    Tracked::reset();
    Tracked n = make_named();
    tut::show(STD, "Tracked n = make_named()  (NRVO)", Tracked::summary());
    tut::note(STD, "NRVO is optional: some compilers/builds show 1 move there");

    tut::section(STD, "2. node handles: extract + insert");
    PatchMap active;
    active.emplace("piano", "piano-sample");
    active.emplace("strings", "strings-sample");
    PatchMap muted;
    Tracked::reset();
    auto node = active.extract("strings");       // unlink the node
    node.key() = "strings (muted)";              // even the KEY can change
    muted.insert(std::move(node));               // relink it in another map
    tut::show(STD, "extract, rename key, insert", Tracked::summary());
    tut::show(STD, "active.size() / muted.size()", std::to_string(active.size()) + " / " + std::to_string(muted.size()));
    tut::show(STD, "muted.begin()->first", muted.begin()->first);

    tut::section(STD, "3. try_emplace doesn't steal when the key exists");
    Tracked candidate("new piano sample");
    Tracked::reset();
    auto [where, inserted] = active.try_emplace("piano", std::move(candidate));
    tut::show(STD, "try_emplace(existing key, std::move(t))", Tracked::summary());
    tut::show(STD, "inserted?", inserted);
    tut::show(STD, "candidate.name() - untouched", candidate.name());
    tut::show(STD, "map still has", where->second.name());

    tut::section(STD, "4. fold expression forwarding a pack");
    std::vector<Tracked> v;
    v.reserve(3);
    Tracked lv("lvalue");
    Tracked::reset();
    push_all(v, lv, Tracked("temp"), std::move(lv));
    tut::show(STD, "push_all(v, lvalue, temp, std::move(lvalue))", Tracked::summary());

    tut::section(STD, "5. std::optional of a move-only type");
    std::optional<std::unique_ptr<int>> maybe = std::make_unique<int>(440);
    std::optional<std::unique_ptr<int>> taken = std::move(maybe);
    tut::show(STD, "*taken.value()", **taken);
    tut::show(STD, "maybe.has_value() after the move", maybe.has_value());
    tut::note(STD, "moving an optional moves its VALUE; the source stays engaged (holding nullptr)");
}

} // namespace l07_cpp17_guaranteed_elision

#endif // TUTORIAL_INCLUDE(17)
