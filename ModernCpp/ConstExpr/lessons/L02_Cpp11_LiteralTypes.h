// =============================================================================
// L02 [C++11]  literal types: constexpr constructors and member functions
// =============================================================================
// Your own classes can be used at compile time if they are LITERAL TYPES:
//   * at least one constexpr constructor (that isn't copy/move)
//   * a trivial destructor (C++11..17; C++20 allows constexpr destructors)
//   * all non-static data members and bases are literal types
//
// C++11 rules for the class's constexpr functions:
//   * constexpr constructor: the BODY must be empty - initialise everything in
//     the member initialiser list
//   * constexpr member function: one return statement, and it is IMPLICITLY
//     const. (C++14 removed the implicit const - so always write `const`.)
//   * so nothing can be modified during constant evaluation in C++11
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(11)

#include <algorithm>
#include <array>
#include <limits>

namespace l02_cpp11_literal_types {

const int STD = 11;

// -----------------------------------------------------------------------------
// 1. A literal class: Point
// -----------------------------------------------------------------------------
class Point {
public:
    constexpr Point() : x_(0.0), y_(0.0) {}
    constexpr Point(double x, double y) : x_(x), y_(y) {}   // body MUST be empty in C++11

    constexpr double x() const { return x_; }
    constexpr double y() const { return y_; }

    constexpr Point operator+(const Point& o) const { return Point(x_ + o.x_, y_ + o.y_); }
    constexpr Point scaled(double k) const { return Point(x_ * k, y_ * k); }

    // An ordinary (runtime) mutator is fine in a literal type; it just can't
    // be constexpr in C++11. (In C++14 it can - see L03.)
    void set_x(double x) { x_ = x; }

    // ERROR DEMO [L02-E1]  fails in: C++11 only (valid from C++14)
    // A constexpr member function that modifies the object. In C++11 it is
    // implicitly const and its body must be a single return statement.
#if TUTORIAL_FORCE_ERRORS
    constexpr void reset() { x_ = 0.0; y_ = 0.0; }
#endif

private:
    double x_;
    double y_;
};

constexpr double dot(const Point& a, const Point& b) { return a.x() * b.x() + a.y() * b.y(); }

constexpr Point midpoint(const Point& a, const Point& b)
{
    return Point((a.x() + b.x()) / 2, (a.y() + b.y()) / 2);
}

constexpr Point kOrigin;              // uses the default constexpr constructor
constexpr Point kA(1.0, 2.0);
constexpr Point kB(3.0, 6.0);
constexpr Point kSum = kA + kB;       // operator+ at compile time
constexpr Point kMid = midpoint(kA, kB);

static_assert(kOrigin.x() == 0.0, "");
static_assert(kSum.x() == 4.0 && kSum.y() == 8.0, "");
static_assert(dot(kA, kB) == 15.0, "");
static_assert(kMid.x() == 2.0 && kMid.y() == 4.0, "");
static_assert(kA.scaled(3).y() == 6.0, "");

// ERROR DEMO [L02-E2]  fails in: every standard
// A constexpr object is const: a non-const member function (set_x) can't be
// called on it.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_set_x() { kA.set_x(5.0); }
#endif

// ERROR DEMO [L02-E3]  fails in: every standard
// The constructor is constexpr, but its ARGUMENT comes from a non-constexpr
// function, so the object can't be built at compile time.
#if TUTORIAL_FORCE_ERRORS
inline double runtime_coordinate() { return 2.0; }
constexpr Point kRuntimePoint(runtime_coordinate(), 0.0);
#endif

// ERROR DEMO [L02-E4]  fails in: every standard
// A constexpr function called with a non-constant argument is fine at runtime,
// but its result can't initialise a constexpr variable.
#if TUTORIAL_FORCE_ERRORS
inline void error_demo_scaled()
{
    double scale = 2.0;
    constexpr Point kScaled = kA.scaled(scale);
    (void)kScaled;
}
#endif

// -----------------------------------------------------------------------------
// 2. Literal types compose: Rect is built from Points
// -----------------------------------------------------------------------------
class Rect {
public:
    constexpr Rect(const Point& lo, const Point& hi) : lo_(lo), hi_(hi) {}

    constexpr double width() const { return hi_.x() - lo_.x(); }
    constexpr double height() const { return hi_.y() - lo_.y(); }
    constexpr double area() const { return width() * height(); }
    constexpr bool contains(const Point& p) const
    {
        return p.x() >= lo_.x() && p.x() <= hi_.x() && p.y() >= lo_.y() && p.y() <= hi_.y();
    }

private:
    Point lo_;
    Point hi_;
};

constexpr Rect kScreen(Point(0, 0), Point(4, 3));
static_assert(kScreen.area() == 12.0, "");
static_assert(kScreen.contains(Point(1, 1)), "");
static_assert(!kScreen.contains(Point(5, 1)), "");

// -----------------------------------------------------------------------------
// 3. What is NOT a literal type
// -----------------------------------------------------------------------------
struct NotLiteral {
    int value;
    ~NotLiteral() {}   // user-provided destructor -> not trivial -> not literal (until C++20)
};
// ERROR DEMO [L02-E5]  fails in: every standard
// NotLiteral has a user-provided (non-constexpr) destructor, so it is not a
// literal type and cannot be constexpr.
#if TUTORIAL_FORCE_ERRORS
constexpr NotLiteral kNotLiteral = { 1 };
#endif

// -----------------------------------------------------------------------------
// 4. User-defined literals can be constexpr (C++11)
// -----------------------------------------------------------------------------
constexpr long double kPi = 3.14159265358979323846L;

constexpr long double operator""_deg(long double degrees) { return degrees * kPi / 180.0L; }
constexpr unsigned long long operator""_KiB(unsigned long long v) { return v * 1024ULL; }

constexpr long double kRightAngle = 90.0_deg;
static_assert(4_KiB == 4096ULL, "");
static_assert(kRightAngle > 1.57L && kRightAngle < 1.58L, "");

// -----------------------------------------------------------------------------
// 5. static constexpr data members (and a C++11/14 linker trap)
// -----------------------------------------------------------------------------
struct AudioConfig {
    static constexpr int kSampleRate = 48000;
    static constexpr int kChannels = 2;
    static constexpr double kGain = 0.5;   // const static double could NOT be initialised in-class; constexpr can
};
static_assert(AudioConfig::kSampleRate * AudioConfig::kChannels == 96000, "");

// In C++11/14 the line above is only a DECLARATION. If the member is
// "odr-used" (e.g. bound to a const& as std::max does), you also need ONE
// out-of-class definition, or you get a LINKER error.
// In C++17 static constexpr members are implicitly inline (see L05), so the
// definition is redundant (and deprecated), hence the guard.
// run() below odr-uses kSampleRate (std::max) and kGain (tut::show takes const T&).
//
// TRY THIS (C++11/14 LINKER error, fine from C++17): comment out the two
// definitions below and build with TUTORIAL_FORCE_ERRORS 0. GCC/Clang report
// "undefined reference to AudioConfig::kGain". (This one can't be a forced
// error demo: link errors only appear when compiling succeeded.)
// MSVC in C++14 mode may behave differently.
#if TUTORIAL_COMPILER_STD < 17
constexpr int AudioConfig::kSampleRate;
constexpr double AudioConfig::kGain;
#endif

// -----------------------------------------------------------------------------
// 6. The C++11 standard library at compile time (very little!)
// -----------------------------------------------------------------------------
static_assert(std::numeric_limits<unsigned char>::max() == 255, "numeric_limits is constexpr");

constexpr std::array<int, 3> kTriad = {{60, 64, 67}};
static_assert(kTriad.size() == 3, "array::size() is constexpr in C++11");

// Note: officially std::array's const operator[] only became constexpr in
// C++14, but GCC's library already allows kTriad[0] in a constant expression
// in C++11 mode. Standard libraries sometimes add constexpr early.

// ERROR DEMO [L02-E6]  fails in: C++11 only (valid from C++14)
// std::max is not constexpr in C++11 (it is from C++14 - see L03).
#if TUTORIAL_FORCE_ERRORS
static_assert(std::max(3, 7) == 7, "");
#endif
// static_assert(kTriad[0] == 60, "");

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L02", "literal types");

    tut::section(STD, "1. constexpr Point");
    tut::show(STD, "kSum = kA + kB  -> x", kSum.x());
    tut::show(STD, "kSum = kA + kB  -> y", kSum.y());
    tut::show(STD, "dot(kA, kB)", dot(kA, kB));
    tut::show(STD, "midpoint(kA, kB).y()", kMid.y());

    Point p(1.0, 1.0);   // a literal type is still a normal runtime type
    p.set_x(10.0);
    tut::show(STD, "runtime Point after set_x(10)", p.x());

    tut::section(STD, "2. composition: Rect of Points");
    tut::show(STD, "kScreen.area()", kScreen.area());
    tut::show(STD, "kScreen.contains(1,1)", kScreen.contains(Point(1, 1)));

    tut::section(STD, "4. constexpr user-defined literals");
    tut::show(STD, "90.0_deg (radians)", static_cast<double>(kRightAngle));
    tut::show(STD, "4_KiB", 4_KiB);

    tut::section(STD, "5. static constexpr members");
    // std::max takes const int& -> odr-use -> needs a definition before C++17
    tut::show(STD, "std::max(kSampleRate, 44100)", std::max(AudioConfig::kSampleRate, 44100));
    tut::show(STD, "AudioConfig::kGain", AudioConfig::kGain);

    tut::section(STD, "6. standard library");
    tut::show(STD, "numeric_limits<uchar>::max()", static_cast<int>(std::numeric_limits<unsigned char>::max()));
    tut::show(STD, "kTriad.size()", kTriad.size());
}

} // namespace l02_cpp11_literal_types

#endif // TUTORIAL_INCLUDE(11)
