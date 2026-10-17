// =============================================================================
// L09 [C++20]  memory allocation at compile time: new/delete, vector, string
// =============================================================================
// C++20 allows "transient" dynamic allocation in constant evaluation:
//   * new / delete, std::allocator, and therefore std::vector and std::string
//   * constexpr destructors (a literal type may now have a non-trivial
//     destructor, as long as it is constexpr)
//   * most of <algorithm> is constexpr: std::sort, std::find, std::copy ...
//
// THE RULE: everything allocated during a constant evaluation must be freed
// before that evaluation ends. Memory can never "leak" from compile time into
// the running program. So
//     constexpr std::vector<int> v = {1, 2, 3};   // error: v would own heap memory
// is NOT allowed, but a constexpr function may build a vector, use it, and
// return a size, a number, or a copy in a std::array.
//
// Library support: MSVC (VS 2019 16.10+), GCC 12+, Clang 15+ with libstdc++ 12+
// or libc++ 15+. Checked below with __cpp_lib_constexpr_vector / _string.
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(20)

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace l09_cpp20_constexpr_allocation {

const int STD = 20;

// -----------------------------------------------------------------------------
// 1. new / delete inside a constexpr function
// -----------------------------------------------------------------------------
constexpr int sum_of_squares(int n)
{
    int* buffer = new int[n];   // allocated during constant evaluation...
    for (int i = 0; i < n; ++i)
        buffer[i] = (i + 1) * (i + 1);
    int total = 0;
    for (int i = 0; i < n; ++i)
        total += buffer[i];
    delete[] buffer;            // ...and freed before it ends
    return total;
}
static_assert(sum_of_squares(10) == 385);

// ERROR DEMO [L09-E1]  fails in: every standard
// Forgetting delete: the allocation outlives the constant evaluation.
#if TUTORIAL_FORCE_ERRORS
constexpr int leaky() { int* p = new int(5); return *p; }
static_assert(leaky() == 5);
#endif

// ERROR DEMO [L09-E2]  fails in: every standard
// Use after free is undefined behaviour - caught during constant evaluation.
#if TUTORIAL_FORCE_ERRORS
constexpr int use_after_free()
{
    int* p = new int(1);
    delete p;
    return *p;
}
static_assert(use_after_free() == 1);
#endif

// -----------------------------------------------------------------------------
// 2. constexpr destructors: RAII at compile time
// -----------------------------------------------------------------------------
class Buffer {
public:
    constexpr explicit Buffer(int size) : data_(new int[size]{}), size_(size) {}
    constexpr ~Buffer() { delete[] data_; }   // C++20: constexpr destructor
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    constexpr int& operator[](int i) { return data_[i]; }
    constexpr int size() const { return size_; }

private:
    int* data_;
    int size_;
};

constexpr int fill_and_sum(int n)
{
    Buffer b(n);   // the destructor frees the memory automatically
    for (int i = 0; i < b.size(); ++i)
        b[i] = i;
    int s = 0;
    for (int i = 0; i < b.size(); ++i)
        s += b[i];
    return s;
}
static_assert(fill_and_sum(100) == 4950);

// -----------------------------------------------------------------------------
// 3. std::vector and std::algorithm at compile time
// -----------------------------------------------------------------------------
#if defined(__cpp_lib_constexpr_vector) && __cpp_lib_constexpr_vector >= 201907L && \
    defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
#define L09_HAS_CONSTEXPR_VECTOR 1

// Sieve of Eratosthenes with a vector<bool>, returning the count.
constexpr std::size_t count_primes_below(int limit)
{
    std::vector<bool> composite(limit, false);
    std::size_t count = 0;
    for (int i = 2; i < limit; ++i) {
        if (composite[i])
            continue;
        ++count;
        for (int j = i * 2; j < limit; j += i)
            composite[j] = true;
    }
    return count;
}
static_assert(count_primes_below(100) == 25);

// Median of a list: copy into a vector, std::sort it, pick the middle.
constexpr int median(std::initializer_list<int> values)
{
    std::vector<int> v(values);
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}
static_assert(median({64, 60, 72, 67, 69}) == 67);

// Getting a vector's contents OUT of compile time: the two-step idiom.
// Step 1: run the computation once to learn the size.
// Step 2: run it again and copy the results into a std::array of that size.
constexpr std::vector<int> primes_below(int limit)
{
    std::vector<int> primes;
    for (int n = 2; n < limit; ++n) {
        bool isPrime = true;
        for (int p : primes)
            if (n % p == 0) {
                isPrime = false;
                break;
            }
        if (isPrime)
            primes.push_back(n);
    }
    return primes;   // fine: the CALLER frees it, still inside constant evaluation
}

constexpr std::size_t kPrimeCount = primes_below(50).size();   // temporary freed here
constexpr auto kPrimes = [] {
    std::array<int, kPrimeCount> out{};
    const std::vector<int> v = primes_below(50);
    std::copy(v.begin(), v.end(), out.begin());
    return out;
}();
static_assert(kPrimes.size() == 15 && kPrimes.back() == 47);

// ERROR DEMO [L09-E3]  fails in: every standard
// A constexpr vector variable would keep heap memory alive at runtime.
#if TUTORIAL_FORCE_ERRORS
constexpr std::vector<int> kNotAllowed = {1, 2, 3};
#endif

#else
#define L09_HAS_CONSTEXPR_VECTOR 0
#endif

// -----------------------------------------------------------------------------
// 4. std::string at compile time
// -----------------------------------------------------------------------------
// Note the version check: older libraries define __cpp_lib_constexpr_string
// with a smaller value for constexpr std::char_traits only.
#if defined(__cpp_lib_constexpr_string) && __cpp_lib_constexpr_string >= 201907L
#define L09_HAS_CONSTEXPR_STRING 1

constexpr std::string note_name(int midi)
{
    constexpr const char* kNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return kNames[midi % 12];   // const char* -> std::string, at compile time
}

// Build "C4" by hand: std::to_string isn't constexpr (std::to_chars is in C++23).
constexpr std::string note_with_octave(int midi)
{
    std::string name = note_name(midi);
    const int octave = midi / 12 - 1;
    if (octave < 0)
        name += '-';
    name += static_cast<char>('0' + (octave < 0 ? -octave : octave));
    return name;
}

constexpr std::size_t kMiddleCLength = note_with_octave(60).size();
static_assert(note_with_octave(60) == "C4");
static_assert(note_with_octave(69) == "A4");
static_assert(kMiddleCLength == 2);

#else
#define L09_HAS_CONSTEXPR_STRING 0
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L09", "constexpr allocation: new/delete, vector, string");

    tut::section(STD, "1. new/delete in a constexpr function");
    tut::show(STD, "sum_of_squares(10)", sum_of_squares(10));

    tut::section(STD, "2. constexpr destructor (RAII)");
    tut::show(STD, "fill_and_sum(100)", fill_and_sum(100));

    tut::section(STD, "3. std::vector + std::sort");
#if L09_HAS_CONSTEXPR_VECTOR
    tut::show(STD, "count_primes_below(100)", count_primes_below(100));
    tut::show(STD, "median({64, 60, 72, 67, 69})", median({64, 60, 72, 67, 69}));
    std::string row;
    for (int p : kPrimes)
        row += std::to_string(p) + ' ';
    tut::show(STD, "kPrimes (vector -> std::array)", row);
#else
    tut::note(STD, "not supported by this standard library (__cpp_lib_constexpr_vector)");
#endif

    tut::section(STD, "4. std::string");
#if L09_HAS_CONSTEXPR_STRING
    tut::show(STD, "note_with_octave(60)", note_with_octave(60));
    tut::show(STD, "note_with_octave(69)", note_with_octave(69));
#else
    tut::note(STD, "not supported by this standard library (__cpp_lib_constexpr_string)");
#endif
}

} // namespace l09_cpp20_constexpr_allocation

#endif // TUTORIAL_INCLUDE(20)
