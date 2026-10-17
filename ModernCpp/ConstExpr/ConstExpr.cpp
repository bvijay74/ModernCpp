// =============================================================================
// ConstExpr.cpp  --  constexpr / consteval from C++11 to C++23
// =============================================================================
// Each lesson lives in lessons/Lxx_*.h and is a self-contained chapter:
// read it top to bottom. Every static_assert in a lesson is checked while
// compiling - if the program builds, those values were computed by the compiler.
//
// Which lessons compile depends on the compiler's C++ standard setting.
// Error demos: TUTORIAL_FORCE_ERRORS in TutorialConfig.h.
//
// Running:
//   ConstExpr.exe        run every compiled lesson
//   ConstExpr.exe 3      run only lesson L03
// =============================================================================

#include <cstdlib>
#include <iostream>

#include "Lesson.h"
#include "TutorialConfig.h"

#include "lessons/L01_Cpp11_Basics.h"
#include "lessons/L02_Cpp11_LiteralTypes.h"
#include "lessons/L03_Cpp14_RelaxedConstexpr.h"
#include "lessons/L04_Cpp17_IfConstexpr.h"
#include "lessons/L05_Cpp17_LambdasAndInline.h"
#include "lessons/L06_Cpp20_Consteval.h"
#include "lessons/L07_Cpp20_Constinit.h"
#include "lessons/L08_Cpp20_IsConstantEvaluated.h"
#include "lessons/L09_Cpp20_ConstexprAllocation.h"
#include "lessons/L10_Cpp20_VirtualTryUnion.h"
#include "lessons/L11_Cpp23_IfConsteval.h"
#include "lessons/L12_Cpp23_RelaxedRules.h"
#include "lessons/L13_Pitfalls.h"
#include "lessons/L14_Capstone.h"

namespace {

int g_lessonsRun = 0;
int g_lessonsSkipped = 0;

// Runs lesson `number` unless the command line asked for a different one.
void run_lesson(int number, int only, void (*lesson)())
{
    if (only != 0 && only != number)
        return;
    lesson();
    ++g_lessonsRun;
}

// A lesson the compiler's standard is too old for.
void skipped(int number, int std_)
{
    ++g_lessonsSkipped;
    std::cout << '\n'
              << "---- " << tut::tag(std_) << (number < 10 ? " L0" : " L") << number
              << "  not compiled: needs C++"
              << std_ << ", compiler is C++" << TUTORIAL_COMPILER_STD << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    const int only = argc > 1 ? std::atoi(argv[1]) : 0;
    (void)&skipped;   // silence "unused function" when every lesson is compiled

    std::cout << "#############################################################\n"
              << "#  constexpr tutorial\n"
              << "#  Compiler standard : C++" << TUTORIAL_COMPILER_STD
              << "   (__cplusplus / _MSVC_LANG = " << TUTORIAL_CPLUSPLUS << "L)\n"
              << "#  Lessons compiled  : C++11 ... C++" << TUTORIAL_COMPILER_STD << '\n'
              << "#############################################################\n";

#if TUTORIAL_INCLUDE(11)
    run_lesson(1, only, &l01_cpp11_basics::run);
    run_lesson(2, only, &l02_cpp11_literal_types::run);
#endif

#if TUTORIAL_INCLUDE(14)
    run_lesson(3, only, &l03_cpp14_relaxed::run);
#else
    skipped(3, 14);
#endif

#if TUTORIAL_INCLUDE(17)
    run_lesson(4, only, &l04_cpp17_if_constexpr::run);
    run_lesson(5, only, &l05_cpp17_lambdas_inline::run);
#else
    skipped(4, 17);
    skipped(5, 17);
#endif

#if TUTORIAL_INCLUDE(20)
    run_lesson(6, only, &l06_cpp20_consteval::run);
    run_lesson(7, only, &l07_cpp20_constinit::run);
    run_lesson(8, only, &l08_cpp20_is_constant_evaluated::run);
    run_lesson(9, only, &l09_cpp20_constexpr_allocation::run);
    run_lesson(10, only, &l10_cpp20_virtual_try_union::run);
#else
    for (int n = 6; n <= 10; ++n)
        skipped(n, 20);
#endif

#if TUTORIAL_INCLUDE(23)
    run_lesson(11, only, &l11_cpp23_if_consteval::run);
    run_lesson(12, only, &l12_cpp23_relaxed_rules::run);
#else
    skipped(11, 23);
    skipped(12, 23);
#endif

#if TUTORIAL_INCLUDE(20)
    run_lesson(13, only, &l13_pitfalls::run);
    run_lesson(14, only, &l14_capstone::run);
#else
    skipped(13, 20);
    skipped(14, 20);
#endif

    std::cout << "\n#############################################################\n"
              << "#  Lessons run: " << g_lessonsRun << "   not compiled: " << g_lessonsSkipped
              << "   (compiler C++" << TUTORIAL_COMPILER_STD << ")\n"
              << "#############################################################\n";
    return 0;
}
