// =============================================================================
// Tracked.h  --  a type that reports every copy and move
// =============================================================================
// Move semantics are invisible: the only difference between a copy and a move
// is what happens inside the constructor. Tracked makes that visible.
//
//   Tracked a("a");            // named object
//   Tracked b = a;             // COUNTED as a copy
//   Tracked c = std::move(a);  // counted as a move; a's name becomes "<moved-from>"
//
//   Tracked::reset();                    // zero the counters
//   ... code under test ...
//   Tracked::summary()   -> "1 copy, 2 moves"
//
//   Tracked::trace(true);                // also print each operation as it happens
//
// This file must compile as C++11, so it only uses C++11 features.
// =============================================================================
#pragma once

#include <iostream>
#include <string>
#include <utility>

namespace tut {

class Tracked {
public:
    struct Stats {
        int copies;
        int moves;
    };

    explicit Tracked(std::string name = "unnamed") : name_(std::move(name)) {}

    Tracked(const Tracked& other) : name_(other.name_)
    {
        ++stats().copies;
        log("COPY construct", name_);
    }

    // noexcept matters: std::vector only moves elements during reallocation if
    // the move constructor can't throw (L03).
    Tracked(Tracked&& other) noexcept : name_(std::move(other.name_))
    {
        other.name_ = "<moved-from>";
        ++stats().moves;
        log("move construct", name_);
    }

    Tracked& operator=(const Tracked& other)
    {
        name_ = other.name_;
        ++stats().copies;
        log("COPY assign", name_);
        return *this;
    }

    Tracked& operator=(Tracked&& other) noexcept
    {
        if (this != &other) {
            name_ = std::move(other.name_);
            other.name_ = "<moved-from>";
        }
        ++stats().moves;
        log("move assign", name_);
        return *this;
    }

    const std::string& name() const { return name_; }

    // ---- counters ----------------------------------------------------------
    static Stats& stats()
    {
        static Stats s = {0, 0};
        return s;
    }
    static void reset() { stats().copies = stats().moves = 0; }
    static int copies() { return stats().copies; }
    static int moves() { return stats().moves; }

    // "0 copies, 1 move"
    static std::string summary()
    {
        const int c = copies();
        const int m = moves();
        return std::to_string(c) + (c == 1 ? " copy, " : " copies, ") + std::to_string(m) +
               (m == 1 ? " move" : " moves");
    }

    static bool& tracing()
    {
        static bool on = false;
        return on;
    }
    static void trace(bool on) { tracing() = on; }

private:
    static void log(const char* what, const std::string& name)
    {
        if (tracing())
            std::cout << "                 > " << what << " \"" << name << "\"\n";
    }

    std::string name_;
};

} // namespace tut
