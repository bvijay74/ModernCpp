// =============================================================================
// L05 [C++11]  the move-aware standard library
// =============================================================================
// C++11 rebuilt the library around move semantics:
//   * std::unique_ptr: single ownership, move-only (replaces std::auto_ptr,
//     whose "copy" secretly moved - removed in C++17)
//   * every container: moving it is O(1) - it just takes the other's buffer
//   * std::swap is three moves instead of three copies
//   * std::move (the algorithm, in <algorithm>) and std::make_move_iterator
//     move a whole range of elements
//   * containers of move-only types: std::vector<std::unique_ptr<T>>
//
// Lambdas in C++11 can only capture by copy or by reference, so a move-only
// object can't be moved INTO a lambda. C++14 fixes that (L06).
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(11)

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace l05_cpp11_move_aware_library {

const int STD = 11;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. std::unique_ptr: ownership you can hand over
// -----------------------------------------------------------------------------
struct Synth {
    std::string name;
    explicit Synth(std::string n) : name(std::move(n)) {}
};

// Returning a unique_ptr by value moves ownership to the caller.
inline std::unique_ptr<Synth> load_synth(const std::string& name)
{
    std::unique_ptr<Synth> s(new Synth(name));
    return s;   // implicit move of a local
}

// Taking a unique_ptr BY VALUE says "I take ownership".
inline std::string adopt(std::unique_ptr<Synth> s) { return s->name + " (now owned by adopt)"; }

// ERROR DEMO [L05-E1]  fails in: every standard
// A vector of unique_ptr can be moved but not copied.
#if TUTORIAL_FORCE_ERRORS
void error_demo_copy_vector_of_unique()
{
    std::vector<std::unique_ptr<int>> a;
    std::vector<std::unique_ptr<int>> b = a;
}
#endif

// ERROR DEMO [L05-E2]  fails in: every standard
// Capturing a unique_ptr BY COPY in a lambda needs its (deleted) copy constructor.
#if TUTORIAL_FORCE_ERRORS
void error_demo_capture_copy()
{
    std::unique_ptr<int> p(new int(1));
    auto f = [p] { return *p; };
    (void)f;
}
#endif

// Moving INTO a lambda needs C++14's init-capture: [q = std::move(p)] (L06).
// (GCC and Clang accept it in C++11 mode too, with a warning, as an extension.)

// ERROR DEMO [L05-E3]  fails in: C++11 only (valid from C++14 - see L06)
// std::make_unique was forgotten in C++11 and added in C++14.
#if TUTORIAL_FORCE_ERRORS
std::unique_ptr<Synth> error_demo_make_unique() { return std::make_unique<Synth>("Moog"); }
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L05", "the move-aware standard library");

    tut::section(STD, "1. std::unique_ptr: transferring ownership");
    std::unique_ptr<Synth> lead = load_synth("Moog");
    tut::show(STD, "load_synth(\"Moog\")->name", lead->name);
    tut::show(STD, "adopt(std::move(lead))", adopt(std::move(lead)));
    tut::show(STD, "lead is empty afterwards", lead == nullptr);

    std::vector<std::unique_ptr<Synth>> rack;
    rack.push_back(load_synth("Prophet"));
    rack.push_back(std::unique_ptr<Synth>(new Synth("Juno")));
    std::vector<std::unique_ptr<Synth>> rack2 = std::move(rack);   // moves the whole vector
    tut::show(STD, "rack2.size() after moving rack", rack2.size());
    tut::show(STD, "rack.size() after the move", rack.size());

    tut::section(STD, "2. moving a container is O(1)");
    std::vector<float> samples(1000000, 0.0f);
    const float* before = samples.data();
    std::vector<float> taken = std::move(samples);
    tut::show(STD, "same buffer after the move?", taken.data() == before);

    tut::section(STD, "3. std::swap uses moves");
    Tracked x("x");
    Tracked y("y");
    Tracked::reset();
    std::swap(x, y);
    tut::show(STD, "std::swap(x, y)", Tracked::summary());
    tut::show(STD, "x.name() after swap", x.name());

    tut::section(STD, "4. moving a range of elements");
    std::vector<Tracked> source;
    source.reserve(3);
    source.emplace_back("C");
    source.emplace_back("E");
    source.emplace_back("G");

    std::vector<Tracked> copyTarget;
    copyTarget.reserve(3);
    Tracked::reset();
    std::copy(source.begin(), source.end(), std::back_inserter(copyTarget));
    tut::show(STD, "std::copy into another vector", Tracked::summary());

    std::vector<Tracked> moveTarget;
    moveTarget.reserve(3);
    Tracked::reset();
    std::move(source.begin(), source.end(), std::back_inserter(moveTarget));
    tut::show(STD, "std::move (algorithm)", Tracked::summary());

    Tracked::reset();
    std::vector<Tracked> built(std::make_move_iterator(moveTarget.begin()),
                               std::make_move_iterator(moveTarget.end()));
    tut::show(STD, "vector(make_move_iterator...)", Tracked::summary());
    tut::show(STD, "source[0].name() afterwards", source[0].name());
}

} // namespace l05_cpp11_move_aware_library

#endif // TUTORIAL_INCLUDE(11)
