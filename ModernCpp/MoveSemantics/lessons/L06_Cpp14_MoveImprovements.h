// =============================================================================
// L06 [C++14]  init-capture, std::exchange, make_unique, decltype(auto)
// =============================================================================
// C++14 filled the gaps C++11 left:
//   * generalised lambda capture ("init-capture"): [p = std::move(ptr)]
//     moves an object INTO a lambda (compare L05-E2)
//   * std::make_unique<T>(args...) - the forwarding factory from L04, standard
//   * std::exchange(obj, newValue): set a new value, return the old one -
//     a one-line way to write move constructors and move assignments
//   * decltype(auto): return EXACTLY what an expression gives, reference and
//     all - needed to forward a return value perfectly
//   * generic lambdas: [](auto&& x) { f(std::forward<decltype(x)>(x)); }
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(14)

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l06_cpp14_move_improvements {

const int STD = 14;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. Init-capture: moving into a lambda
// -----------------------------------------------------------------------------
struct Recording {
    std::string title;
    std::vector<float> samples;
};

inline auto make_player(std::unique_ptr<Recording> rec)
{
    // The lambda now OWNS the recording.
    return [r = std::move(rec)] { return "playing \"" + r->title + "\""; };
}

// ERROR DEMO [L06-E1]  fails in: every standard
// std::function requires a COPYABLE callable, and this lambda owns a
// unique_ptr, so it is move-only. C++23's std::move_only_function fixes this (L09).
#if TUTORIAL_FORCE_ERRORS
std::function<std::string()> error_demo_function()
{
    return make_player(std::make_unique<Recording>());
}
#endif

// A lambda's captures are const unless the lambda is `mutable`. To move a
// capture OUT of the lambda, it has to be mutable.
inline auto make_one_shot(Tracked t)
{
    return [t = std::move(t)]() mutable { return std::move(t); };
}

// ERROR DEMO [L06-E2]  fails in: every standard
// Without `mutable`, the capture is const: std::move(p) is a const&& and
// can't move a unique_ptr out.
#if TUTORIAL_FORCE_ERRORS
std::unique_ptr<int> error_demo_not_mutable()
{
    auto f = [p = std::make_unique<int>(1)] { return std::move(p); };
    return f();
}
#endif

// -----------------------------------------------------------------------------
// 2. std::exchange: tidy move operations
// -----------------------------------------------------------------------------
class SampleBuffer {
public:
    explicit SampleBuffer(std::size_t n) : size_(n), data_(new float[n]()) {}
    ~SampleBuffer() { delete[] data_; }
    SampleBuffer(const SampleBuffer&) = delete;
    SampleBuffer& operator=(const SampleBuffer&) = delete;

    // Compare with the C++11 version in L02: one line per member.
    SampleBuffer(SampleBuffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)), data_(std::exchange(other.data_, nullptr))
    {
    }
    SampleBuffer& operator=(SampleBuffer&& other) noexcept
    {
        delete[] std::exchange(data_, std::exchange(other.data_, nullptr));
        size_ = std::exchange(other.size_, 0);
        return *this;
    }

    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    float* data_;
};

// -----------------------------------------------------------------------------
// 3. decltype(auto): forwarding a return value
// -----------------------------------------------------------------------------
struct Mixer {
    std::vector<std::string> channels{"Piano", "Bass", "Drums"};
    std::string& channel(std::size_t i) { return channels[i]; }   // returns a reference
};

// `auto` would always return a copy; decltype(auto) keeps the reference.
template <typename F, typename... Args>
decltype(auto) call_and_log(F&& f, Args&&... args)
{
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto call_by_value(F&& f, Args&&... args)
{
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

// -----------------------------------------------------------------------------
// 4. Generic lambdas that forward
// -----------------------------------------------------------------------------
inline void keep(const Tracked& t) { Tracked kept(t); }
inline void keep(Tracked&& t) { Tracked kept(std::move(t)); }

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L06", "init-capture, std::exchange, make_unique, decltype(auto)");

    tut::section(STD, "1. init-capture: a lambda that owns its data");
    auto rec = std::make_unique<Recording>();
    rec->title = "Moonlight Sonata";
    auto player = make_player(std::move(rec));
    tut::show(STD, "player()", player());
    tut::show(STD, "rec is empty after the move", rec == nullptr);

    Tracked::reset();
    auto oneShot = make_one_shot(Tracked("sample"));
    Tracked out = oneShot();   // moves the capture out
    tut::show(STD, "make_one_shot + call", Tracked::summary());
    tut::show(STD, "out.name()", out.name());

    tut::section(STD, "2. std::exchange in move operations");
    SampleBuffer a(1024);
    SampleBuffer b = std::move(a);
    tut::show(STD, "b.size() after the move", b.size());
    tut::show(STD, "a.size() after the move", a.size());
    int tempo = 120;
    const int oldTempo = std::exchange(tempo, 96);
    tut::show(STD, "std::exchange(tempo, 96): old / new", std::to_string(oldTempo) + " / " + std::to_string(tempo));

    tut::section(STD, "3. decltype(auto) keeps references");
    Mixer m;
    auto getChannel = [&m](std::size_t i) -> std::string& { return m.channel(i); };
    call_and_log(getChannel, 0) = "Grand Piano";    // a real reference: modifies the mixer
    call_by_value(getChannel, 1) = "Synth Bass";    // only modifies a temporary copy
    tut::show(STD, "via decltype(auto): channels[0]", m.channels[0]);
    tut::show(STD, "via auto:           channels[1]", m.channels[1]);
    tut::show(STD, "returns a reference? decltype(auto)",
              std::is_lvalue_reference<decltype(call_and_log(getChannel, 0))>::value);
    tut::show(STD, "returns a reference? auto",
              std::is_lvalue_reference<decltype(call_by_value(getChannel, 0))>::value);

    tut::section(STD, "4. generic lambda with std::forward<decltype(x)>");
    auto forwarder = [](auto&& x) { keep(std::forward<decltype(x)>(x)); };
    Tracked lv("lvalue");
    Tracked::reset();
    forwarder(lv);
    tut::show(STD, "forwarder(lvalue)", Tracked::summary());
    Tracked::reset();
    forwarder(Tracked("temp"));
    tut::show(STD, "forwarder(temporary)", Tracked::summary());
}

} // namespace l06_cpp14_move_improvements

#endif // TUTORIAL_INCLUDE(14)
