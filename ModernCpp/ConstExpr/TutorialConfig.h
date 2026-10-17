// =============================================================================
// TutorialConfig.h
// =============================================================================
//
// 1. WHICH LESSONS COMPILE: the compiler's C++ standard decides.
//
//    Project Properties > C/C++ > Language > C++ Language Standard
//
//      C++14  ->  L01-L03            (C++11, C++14)
//      C++17  ->  L01-L05            (+ C++17)
//      C++20  ->  L01-L10, L13, L14  (+ C++20)
//      C++23  ->  L01-L14            (+ C++23)  /std:c++latest in Visual Studio
//
//    Some C++23 features aren't in every compiler yet. Those parts check a
//    feature-test macro (e.g. __cpp_if_consteval) and print "not supported"
//    instead of failing to build.
//
//    Visual Studio has no C++11 setting (its oldest is C++14). To compile as
//    C++11, use GCC or Clang, e.g. in WSL or on https://godbolt.org:
//        g++ -std=c++11 ConstExpr.cpp
//
// 2. FORCED ERRORS: the one switch you edit (below).
//
//    0 = build and run normally
//    1 = also compile the ERROR DEMO snippets, so you can read the compiler's
//        message for each constexpr mistake
//
//    Every demo sits next to the code it is about, is labelled [L01-E1],
//    [L01-E2], ... and says which standards it fails in:
//      "fails in: every standard"  always an error
//      "fails in: C++11 only"      an error under that compiler setting, but it
//                                  compiles cleanly under a newer one. Build it
//                                  with both settings to see the rule change.
// =============================================================================
#pragma once

#ifndef TUTORIAL_FORCE_ERRORS
#define TUTORIAL_FORCE_ERRORS 0
#endif

// ---- Nothing below needs editing --------------------------------------------

// Which standard is the compiler using?
// MSVC leaves __cplusplus at 199711L unless /Zc:__cplusplus is set, but always
// reports the real value in _MSVC_LANG, so prefer that.
#if defined(_MSVC_LANG)
#define TUTORIAL_CPLUSPLUS _MSVC_LANG
#else
#define TUTORIAL_CPLUSPLUS __cplusplus
#endif

// Pre-release modes (e.g. /std:c++latest) report a value just above the last
// published standard, so compare with ">" against the previous standard.
#if TUTORIAL_CPLUSPLUS > 202302L
#define TUTORIAL_COMPILER_STD 26
#elif TUTORIAL_CPLUSPLUS > 202002L
#define TUTORIAL_COMPILER_STD 23
#elif TUTORIAL_CPLUSPLUS > 201703L
#define TUTORIAL_COMPILER_STD 20
#elif TUTORIAL_CPLUSPLUS > 201402L
#define TUTORIAL_COMPILER_STD 17
#elif TUTORIAL_CPLUSPLUS > 201103L
#define TUTORIAL_COMPILER_STD 14
#else
#define TUTORIAL_COMPILER_STD 11
#endif

// A lesson for standard `std_` is compiled when the compiler supports it.
#define TUTORIAL_INCLUDE(std_) ((std_) <= TUTORIAL_COMPILER_STD)

#define TUTORIAL_STR2(x) #x
#define TUTORIAL_STR(x) TUTORIAL_STR2(x)
#if TUTORIAL_FORCE_ERRORS
#pragma message("ConstExpr tutorial: TUTORIAL_FORCE_ERRORS is 1 (compiler: C++" \
                TUTORIAL_STR(TUTORIAL_COMPILER_STD)                              \
                "). The errors labelled [Lxx-En] are INTENTIONAL - set it to 0 to build normally.")
#endif
