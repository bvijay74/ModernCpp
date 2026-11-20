// =============================================================================
// L02 [C++11]  move constructor and move assignment
// =============================================================================
// A class that owns a resource (heap memory, a file, a socket) can now have
// two more special member functions:
//
//   Buffer(Buffer&& other) noexcept;              // move constructor
//   Buffer& operator=(Buffer&& other) noexcept;   // move assignment
//
// A COPY duplicates the resource (allocate + copy every element).
// A MOVE steals it: copy the pointer, then leave `other` empty but valid.
//
// RULE OF FIVE: if you write any of destructor / copy ctor / copy assignment /
// move ctor / move assignment, think about all five.
// RULE OF ZERO: better still, hold resources in members that already manage
// themselves (std::vector, std::string, std::unique_ptr) and write none.
//
// When does the compiler generate move operations for you? Only if you have
// NOT declared a destructor, copy constructor, copy assignment or the other
// move operation. Declaring an empty destructor silently turns every "move"
// of your class into a copy (section 4).
// =============================================================================
#pragma once

#include "../Lesson.h"
#include "../Tracked.h"

#if TUTORIAL_INCLUDE(11)

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace l02_cpp11_move_operations {

const int STD = 11;
using tut::Tracked;

// -----------------------------------------------------------------------------
// 1. Rule of five by hand: an audio sample buffer
// -----------------------------------------------------------------------------
class Buffer {
public:
    explicit Buffer(std::size_t frames) : size_(frames), data_(allocate(frames)) {}

    ~Buffer() { delete[] data_; }

    // Copy: allocate new memory and copy every sample.
    Buffer(const Buffer& other) : size_(other.size_), data_(allocate(other.size_))
    {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // Move: take the pointer, leave `other` empty. No allocation, can't throw.
    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_)
    {
        other.size_ = 0;
        other.data_ = nullptr;
    }

    // Copy assignment via copy-and-swap: strong exception guarantee.
    Buffer& operator=(const Buffer& other)
    {
        Buffer copy(other);
        swap(copy);
        return *this;
    }

    // Move assignment: release ours, take theirs. The self-check keeps
    // `b = std::move(b);` from deleting the data we're about to keep.
    Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(Buffer& other) noexcept
    {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    std::size_t size() const { return size_; }
    const float* data() const { return data_; }
    float& operator[](std::size_t i) { return data_[i]; }

    static int& allocations()
    {
        static int n = 0;
        return n;
    }

private:
    static float* allocate(std::size_t n)
    {
        if (n == 0)
            return nullptr;
        ++allocations();
        return new float[n]();
    }

    std::size_t size_;
    float* data_;
};

// -----------------------------------------------------------------------------
// 2. Rule of zero: let the members do it
// -----------------------------------------------------------------------------
struct Track {
    std::string name;
    std::vector<float> samples;
    Tracked tag{"track-tag"};   // so we can watch the generated moves
};

// -----------------------------------------------------------------------------
// 3. Move-only types: delete the copy operations
// -----------------------------------------------------------------------------
class DeviceHandle {
public:
    explicit DeviceHandle(int id) : id_(id) {}
    DeviceHandle(const DeviceHandle&) = delete;
    DeviceHandle& operator=(const DeviceHandle&) = delete;
    DeviceHandle(DeviceHandle&& other) noexcept : id_(other.id_) { other.id_ = -1; }
    DeviceHandle& operator=(DeviceHandle&& other) noexcept
    {
        std::swap(id_, other.id_);
        return *this;
    }
    int id() const { return id_; }

private:
    int id_;
};

// ERROR DEMO [L02-E1]  fails in: every standard
// Copying a move-only type: the copy constructor is deleted.
#if TUTORIAL_FORCE_ERRORS
void error_demo_copy_move_only()
{
    DeviceHandle a(1);
    DeviceHandle b = a;
    (void)b;
}
#endif

// A type that can be neither copied nor moved.
struct Pinned {
    Pinned() {}
    Pinned(const Pinned&) = delete;
    Pinned(Pinned&&) = delete;
};

// ERROR DEMO [L02-E2]  fails in: C++11, C++14 (valid from C++17 - see L07)
// Returning a temporary still formally needs the move constructor before
// C++17, even though every compiler elides the move. C++17 guarantees the
// elision, so the move constructor isn't needed at all.
#if TUTORIAL_FORCE_ERRORS
Pinned make_pinned() { return Pinned(); }
#endif

// -----------------------------------------------------------------------------
// 4. When the compiler does NOT generate move operations
// -----------------------------------------------------------------------------
struct WithDestructor {
    Tracked tag{"with-dtor"};
    ~WithDestructor() {}   // a user-declared destructor suppresses the implicit moves
};

struct WithDefaultedMoves {
    Tracked tag{"defaulted"};
    ~WithDefaultedMoves() {}
    // Bring them back explicitly (and then the copies too - rule of five).
    WithDefaultedMoves() = default;
    WithDefaultedMoves(const WithDefaultedMoves&) = default;
    WithDefaultedMoves& operator=(const WithDefaultedMoves&) = default;
    WithDefaultedMoves(WithDefaultedMoves&&) = default;
    WithDefaultedMoves& operator=(WithDefaultedMoves&&) = default;
};

// ERROR DEMO [L02-E3]  fails in: every standard
// A const data member can't be assigned, so move assignment is deleted.
#if TUTORIAL_FORCE_ERRORS
struct Note {
    const int pitch;
};
void error_demo_const_member()
{
    Note a = {60};
    Note b = {64};
    a = std::move(b);
}
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L02", "move constructor and move assignment");

    tut::section(STD, "1. copy vs move of a resource-owning class");
    Buffer::allocations() = 0;
    Buffer original(48000);   // one second at 48 kHz
    original[0] = 0.5f;
    const float* originalMemory = original.data();

    Buffer copied = original;               // copy constructor
    tut::show(STD, "copy: allocations so far", Buffer::allocations());
    tut::show(STD, "copy: uses the same memory?", copied.data() == originalMemory);

    Buffer moved = std::move(original);     // move constructor
    tut::show(STD, "move: allocations so far", Buffer::allocations());
    tut::show(STD, "move: uses the same memory?", moved.data() == originalMemory);
    tut::show(STD, "moved-from original.size()", original.size());

    original = std::move(moved);            // move assignment back
    tut::show(STD, "after move assignment back: size", original.size());

    tut::section(STD, "2. rule of zero: generated moves");
    Track a;
    a.name = "Piano";
    a.samples.assign(1000, 0.0f);
    Tracked::reset();
    Track b = std::move(a);
    tut::show(STD, "Track b = std::move(a)", Tracked::summary());
    tut::show(STD, "b.samples.size()", b.samples.size());
    tut::show(STD, "a.samples.size() after the move", a.samples.size());

    tut::section(STD, "3. move-only type");
    DeviceHandle h1(7);
    DeviceHandle h2 = std::move(h1);
    tut::show(STD, "h2.id() after the move", h2.id());
    tut::show(STD, "h1.id() after the move", h1.id());

    tut::section(STD, "4. a destructor suppresses the implicit moves");
    WithDestructor w1;
    Tracked::reset();
    WithDestructor w2 = std::move(w1);
    tut::show(STD, "WithDestructor: std::move(w1)", Tracked::summary());
    WithDefaultedMoves d1;
    Tracked::reset();
    WithDefaultedMoves d2 = std::move(d1);
    tut::show(STD, "WithDefaultedMoves: std::move(d1)", Tracked::summary());
    tut::note(STD, "the 'move' of WithDestructor quietly became a copy");
    (void)w2;
    (void)d2;
}

} // namespace l02_cpp11_move_operations

#endif // TUTORIAL_INCLUDE(11)
