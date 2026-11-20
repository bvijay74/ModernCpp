// =============================================================================
// L11 [C++17]  capstone: an audio processing pipeline
// =============================================================================
// A one-second audio buffer is 48000 floats (~188 KB). Processing it through
// a chain of effects shows what move semantics buys in practice:
//
// Part 1  The buffer type: move-only, copies must be explicit (clone())
// Part 2  Two pipelines doing the same work:
//           copy style   AudioBuffer gain(const AudioBuffer& in) -> new buffer per stage
//           move style   AudioBuffer gain(AudioBuffer buf)       -> one buffer, passed along
// Part 3  A mixer: tracks built in place with perfect forwarding
// Part 4  Muting a track: map node handles move it without touching the audio
//
// Features used: move-only types and noexcept moves (C++11), make_unique
// (C++14), node handles, emplace_back returning a reference, structured
// bindings (C++17).
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(17)

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace l11_capstone {

const int STD = 17;

// =============================================================================
// Part 1: a move-only audio buffer
// =============================================================================
class AudioBuffer {
public:
    struct Stats {
        int allocations = 0;
        std::size_t bytes = 0;
    };

    explicit AudioBuffer(std::size_t frames) : frames_(frames), samples_(std::make_unique<float[]>(frames))
    {
        ++stats().allocations;
        stats().bytes += frames * sizeof(float);
    }

    // Move-only: moving hands over the unique_ptr, copying is not allowed
    // implicitly - you must say clone() so every copy is visible in the code.
    AudioBuffer(AudioBuffer&& other) noexcept
        : frames_(std::exchange(other.frames_, 0)), samples_(std::move(other.samples_))
    {
    }
    AudioBuffer& operator=(AudioBuffer&& other) noexcept
    {
        frames_ = std::exchange(other.frames_, 0);
        samples_ = std::move(other.samples_);
        return *this;
    }
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;

    AudioBuffer clone() const
    {
        AudioBuffer copy(frames_);
        std::copy(begin(), end(), copy.begin());
        return copy;   // guaranteed elision / NRVO: no extra work
    }

    float* begin() { return samples_.get(); }
    float* end() { return samples_.get() + frames_; }
    const float* begin() const { return samples_.get(); }
    const float* end() const { return samples_.get() + frames_; }
    std::size_t frames() const { return frames_; }
    float peak() const
    {
        float p = 0.0f;
        for (float s : *this)
            p = std::max(p, std::fabs(s));
        return p;
    }

    static Stats& stats()
    {
        static Stats s;
        return s;
    }
    static void reset_stats() { stats() = Stats{}; }

private:
    std::size_t frames_;
    std::unique_ptr<float[]> samples_;
};

static_assert(std::is_nothrow_move_constructible_v<AudioBuffer>);
static_assert(!std::is_copy_constructible_v<AudioBuffer>);

// ERROR DEMO [L11-E1]  fails in: every standard
// AudioBuffer is move-only: an accidental copy doesn't compile.
#if TUTORIAL_FORCE_ERRORS
void error_demo_copy_buffer()
{
    AudioBuffer a(16);
    AudioBuffer b = a;
}
#endif

constexpr std::size_t kSampleRate = 48000;
constexpr double kPi = 3.14159265358979323846;

inline AudioBuffer sine(double hz, std::size_t frames)
{
    AudioBuffer buf(frames);
    std::size_t i = 0;
    for (float& s : buf)
        s = static_cast<float>(std::sin(2.0 * kPi * hz * static_cast<double>(i++) / kSampleRate));
    return buf;
}

// =============================================================================
// Part 2a: copy-style effects - each stage allocates a new buffer
// =============================================================================
namespace copy_style {
inline AudioBuffer gain(const AudioBuffer& in, float g)
{
    AudioBuffer out = in.clone();
    for (float& s : out)
        s *= g;
    return out;
}
inline AudioBuffer fade_in(const AudioBuffer& in, std::size_t frames)
{
    AudioBuffer out = in.clone();
    for (std::size_t i = 0; i < frames && i < out.frames(); ++i)
        out.begin()[i] *= static_cast<float>(i) / static_cast<float>(frames);
    return out;
}
inline AudioBuffer normalize(const AudioBuffer& in, float target)
{
    AudioBuffer out = in.clone();
    const float p = out.peak();
    if (p > 0.0f)
        for (float& s : out)
            s *= target / p;
    return out;
}
} // namespace copy_style

// =============================================================================
// Part 2b: move-style effects - take by value, modify, return
// =============================================================================
// The caller decides: pass std::move(buf) (or a temporary) and no copy is
// ever made; pass an lvalue and it won't compile - you have to clone()
// explicitly, because AudioBuffer is move-only.
namespace move_style {
inline AudioBuffer gain(AudioBuffer buf, float g)
{
    for (float& s : buf)
        s *= g;
    return buf;   // a by-value parameter is moved out implicitly
}
inline AudioBuffer fade_in(AudioBuffer buf, std::size_t frames)
{
    for (std::size_t i = 0; i < frames && i < buf.frames(); ++i)
        buf.begin()[i] *= static_cast<float>(i) / static_cast<float>(frames);
    return buf;
}
inline AudioBuffer normalize(AudioBuffer buf, float target)
{
    const float p = buf.peak();
    if (p > 0.0f)
        for (float& s : buf)
            s *= target / p;
    return buf;
}
} // namespace move_style

// =============================================================================
// Part 3: a mixer that builds tracks in place
// =============================================================================
struct Track {
    std::string name;
    AudioBuffer audio;
    float pan;
    Track(std::string n, AudioBuffer a, float p) : name(std::move(n)), audio(std::move(a)), pan(p) {}
};
static_assert(std::is_nothrow_move_constructible_v<Track>, "vector<Track> must move when it grows");

class Mixer {
public:
    template <typename... Args>
    Track& add_track(Args&&... args)
    {
        return tracks_.emplace_back(std::forward<Args>(args)...);   // C++17: returns a reference
    }
    const std::vector<Track>& tracks() const { return tracks_; }

private:
    std::vector<Track> tracks_;
};

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L11", "capstone: an audio processing pipeline");
    const std::size_t oneSecond = kSampleRate;

    tut::section(STD, "Part 2: the same three effects, copy style vs move style");
    AudioBuffer::reset_stats();
    {
        AudioBuffer source = sine(440.0, oneSecond);
        AudioBuffer result =
            copy_style::normalize(copy_style::fade_in(copy_style::gain(source, 0.5f), 4800), 0.9f);
        tut::show(STD, "copy style: allocations", AudioBuffer::stats().allocations);
        tut::show(STD, "copy style: KB allocated", AudioBuffer::stats().bytes / 1024);
        tut::show(STD, "copy style: peak", result.peak());
    }
    AudioBuffer::reset_stats();
    {
        AudioBuffer result =
            move_style::normalize(move_style::fade_in(move_style::gain(sine(440.0, oneSecond), 0.5f), 4800), 0.9f);
        tut::show(STD, "move style: allocations", AudioBuffer::stats().allocations);
        tut::show(STD, "move style: KB allocated", AudioBuffer::stats().bytes / 1024);
        tut::show(STD, "move style: peak", result.peak());
    }
    tut::note(STD, "same output, one allocation instead of four");

    tut::section(STD, "Part 3: building tracks in place with perfect forwarding");
    AudioBuffer::reset_stats();
    Mixer mixer;
    mixer.add_track("Melody", sine(523.25, oneSecond), -0.3f);       // C5
    mixer.add_track("Bass", sine(130.81, oneSecond), 0.0f);          // C3
    AudioBuffer harmony = sine(659.25, oneSecond);                   // E5
    Track& h = mixer.add_track("Harmony", std::move(harmony), 0.3f);
    tut::show(STD, "tracks / allocations", std::to_string(mixer.tracks().size()) + " / " +
                                               std::to_string(AudioBuffer::stats().allocations));
    tut::show(STD, "add_track returned a reference to", h.name);
    tut::show(STD, "harmony.frames() after the move", harmony.frames());

    tut::section(STD, "Part 4: muting a track with node handles");
    std::map<std::string, AudioBuffer> active;
    std::map<std::string, AudioBuffer> muted;
    active.emplace("Drums", sine(80.0, oneSecond));
    active.emplace("Strings", sine(392.0, oneSecond));
    const float* audioBefore = active.at("Strings").begin();
    AudioBuffer::reset_stats();

    muted.insert(active.extract("Strings"));   // move the NODE: key and audio untouched

    tut::show(STD, "allocations while muting", AudioBuffer::stats().allocations);
    tut::show(STD, "same audio memory after the move?", muted.at("Strings").begin() == audioBefore);
    for (const auto& [name, audio] : muted)
        tut::show(STD, "muted: " + name, std::to_string(audio.frames()) + " frames");
}

} // namespace l11_capstone

#endif // TUTORIAL_INCLUDE(17)
