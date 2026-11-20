// =============================================================================
// L09 [C++23]  deducing this, forward_like, move_only_function, auto(x)
// =============================================================================
// C++23 changes for move semantics:
//   * SIMPLER IMPLICIT MOVE (P2266): a returned name that refers to a movable
//     object is treated as an rvalue, also when the function returns T&&
//     (compare L08-E1)
//   * DEDUCING THIS (P0847): `this Self&& self` as an explicit first
//     parameter. One member function template replaces the &, const& and &&
//     overloads from L01 section 5 - the object is forwarded like any argument.
//   * std::forward_like<Self>(x): forward x with the value category of
//     another object (usually the owner of x)
//   * std::move_only_function: like std::function, but accepts move-only
//     callables such as a lambda owning a unique_ptr (compare L06-E1)
//   * auto(x): make a prvalue COPY of x explicitly ("decay-copy")
//
// Each part checks its feature-test macro; unsupported parts print a note.
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(23)

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l09_cpp23_move_improvements {

const int STD = 23;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. Simpler implicit move
// -----------------------------------------------------------------------------
#if defined(__cpp_implicit_move) && __cpp_implicit_move >= 202207L
#define L09_HAS_IMPLICIT_MOVE 1
inline Tracked&& give_back(Tracked&& t) { return t; }   // L08-E1 compiles now
#else
#define L09_HAS_IMPLICIT_MOVE 0
#endif

// -----------------------------------------------------------------------------
// 2. Deducing this
// -----------------------------------------------------------------------------
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
#define L09_HAS_DEDUCING_THIS 1
class Patch {
public:
    explicit Patch(std::string sampleName) : sample_(std::move(sampleName)) {}

    // ONE function for every kind of object:
    //   lvalue Patch  -> Self = Patch&       -> returns Tracked&  (caller copies)
    //   rvalue Patch  -> Self = Patch        -> returns Tracked&& (caller moves)
    //   const Patch   -> Self = const Patch& -> returns const Tracked&
    template <typename Self>
    auto&& sample(this Self&& self)
    {
        return std::forward<Self>(self).sample_;
    }

private:
    Tracked sample_;
};

// ERROR DEMO [L09-E1]  fails in: every standard
// An explicit-object member function has no `this` pointer: use `self`.
#if TUTORIAL_FORCE_ERRORS
struct Counter {
    int count = 0;
    void bump(this Counter& self) { ++this->count; }
};
#endif
#else
#define L09_HAS_DEDUCING_THIS 0
#endif

// -----------------------------------------------------------------------------
// 3. std::forward_like
// -----------------------------------------------------------------------------
#if L09_HAS_DEDUCING_THIS && defined(__cpp_lib_forward_like) && __cpp_lib_forward_like >= 202207L
#define L09_HAS_FORWARD_LIKE 1
// The Tracked object lives behind a unique_ptr. std::forward_like gives it
// the value category of the OWNER: an expiring Holder lets you move it out.
class Holder {
public:
    explicit Holder(std::string name) : ptr_(std::make_unique<Tracked>(std::move(name))) {}

    template <typename Self>
    auto&& get(this Self&& self)
    {
        return std::forward_like<Self>(*self.ptr_);
    }

private:
    std::unique_ptr<Tracked> ptr_;
};
#else
#define L09_HAS_FORWARD_LIKE 0
#endif

// -----------------------------------------------------------------------------
// 4. std::move_only_function
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_move_only_function) && __cpp_lib_move_only_function >= 202110L
#define L09_HAS_MOVE_ONLY_FUNCTION 1
inline std::move_only_function<std::string()> make_task(std::unique_ptr<std::string> title)
{
    return [t = std::move(title)] { return "rendering \"" + *t + "\""; };
}

// ERROR DEMO [L09-E2]  fails in: every standard
// As the name says: a move_only_function can't be copied.
#if TUTORIAL_FORCE_ERRORS
void error_demo_copy_task()
{
    auto task = make_task(std::make_unique<std::string>("x"));
    auto copy = task;
}
#endif
#else
#define L09_HAS_MOVE_ONLY_FUNCTION 0
#endif

// -----------------------------------------------------------------------------
// 5. auto(x): an explicit copy as a prvalue
// -----------------------------------------------------------------------------
inline std::string consume(Tracked&& t)
{
    Tracked kept(std::move(t));
    return kept.name();
}

#if defined(__cpp_auto_cast) && __cpp_auto_cast >= 202110L
#define L09_HAS_AUTO_CAST 1
#else
#define L09_HAS_AUTO_CAST 0
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L09", "deducing this, forward_like, move_only_function, auto(x)");
    const char* missing = "skipped: your compiler/library doesn't report this feature (feature-test macro)";
    (void)missing;   // unused when everything is supported

    tut::section(STD, "1. simpler implicit move");
#if L09_HAS_IMPLICIT_MOVE
    Tracked::reset();
    Tracked got = give_back(Tracked("returned"));
    tut::show(STD, "Tracked&& give_back(Tracked&& t) { return t; }", Tracked::summary());
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "2. deducing this");
#if L09_HAS_DEDUCING_THIS
    Patch piano("piano-sample");
    Tracked::reset();
    Tracked a = piano.sample();
    tut::show(STD, "piano.sample()  (lvalue object)", Tracked::summary());
    Tracked::reset();
    Tracked b = std::move(piano).sample();
    tut::show(STD, "std::move(piano).sample()", Tracked::summary());
    Tracked::reset();
    Tracked c = Patch("organ-sample").sample();
    tut::show(STD, "Patch(\"organ-sample\").sample()", Tracked::summary());
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "3. std::forward_like");
#if L09_HAS_FORWARD_LIKE
    Holder h("held");
    Tracked::reset();
    Tracked fromLvalue = h.get();
    tut::show(STD, "h.get()  (lvalue holder)", Tracked::summary());
    Tracked::reset();
    Tracked fromRvalue = std::move(h).get();
    tut::show(STD, "std::move(h).get()", Tracked::summary());
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "4. std::move_only_function");
#if L09_HAS_MOVE_ONLY_FUNCTION
    auto task = make_task(std::make_unique<std::string>("Symphony No. 5"));
    std::vector<std::move_only_function<std::string()>> queue;
    queue.push_back(std::move(task));
    tut::show(STD, "queue[0]()", queue[0]());
#else
    tut::note(STD, missing);
#endif

    tut::section(STD, "5. auto(x)");
#if L09_HAS_AUTO_CAST
    Tracked original("original");
    Tracked::reset();
    std::string result = consume(auto(original));   // pass a fresh copy as an rvalue
    tut::show(STD, "consume(auto(original))", Tracked::summary());
    tut::show(STD, "original.name() - untouched", original.name());
    tut::note(STD, "before C++23: consume(Tracked(original)) - auto(x) also works in generic code");
    (void)result;
#else
    tut::note(STD, missing);
#endif
}

} // namespace l09_cpp23_move_improvements

#endif // TUTORIAL_INCLUDE(23)
