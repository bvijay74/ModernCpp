// =============================================================================
// Lesson.h  --  small printing helpers shared by every lesson
// =============================================================================
// Every line of console output is tagged with the C++ standard that introduced
// the feature being shown, e.g.
//
//     [C++14] factorial_14(15) ................ 1307674368000
//
// The banner at the top of the run shows which standard the COMPILER used.
//
// This file must itself compile as C++11, so it only uses C++11 features.
// =============================================================================
#pragma once

#include <iomanip>
#include <iostream>
#include <string>

#include "TutorialConfig.h"

namespace tut {

inline std::string tag(int std_) { return "[C++" + std::to_string(std_) + "]"; }

// "==== [C++11] L01  constexpr basics ====   (compiler: C++17)"
inline void header(int std_, const char* id, const char* title)
{
    std::cout << '\n'
              << "==== " << tag(std_) << ' ' << id << "  " << title << " ====   (compiler: C++"
              << TUTORIAL_COMPILER_STD << ")\n";
}

// "[C++11] -- 2. constexpr functions"
inline void section(int std_, const char* title)
{
    std::cout << tag(std_) << " -- " << title << '\n';
}

// "[C++11]    factorial(10) ............... 3628800"
template <typename T>
void show(int std_, const std::string& label, const T& value)
{
    std::string dots = label + ' ';
    if (dots.size() < 42)
        dots.append(42 - dots.size(), '.');
    std::cout << tag(std_) << "    " << dots << ' ' << value << '\n';
}

inline void show(int std_, const std::string& label, bool value)
{
    show(std_, label, value ? "true" : "false");
}

// "[C++11]    note text"
inline void note(int std_, const std::string& text)
{
    std::cout << tag(std_) << "    " << text << '\n';
}

} // namespace tut
