// =============================================================================
// MoveSemantics.cpp  --  rvalue references, move semantics and perfect
//                        forwarding from C++11 to C++23
// =============================================================================
// Each lesson lives in lessons/Lxx_*.h and is a self-contained chapter: read
// it top to bottom, then run it. The Tracked type (Tracked.h) counts every
// copy and move, so the output shows what each line of code really did.
//
// Which lessons compile depends on the compiler's C++ standard setting.
// Error demos: TUTORIAL_FORCE_ERRORS in TutorialConfig.h.
//
// Running:
//   MoveSemantics.exe        run every compiled lesson
//   MoveSemantics.exe 4      run only lesson L04
//   MoveSemantics.exe 4 -t   ...and trace every copy/move as it happens
// =============================================================================

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Lesson.h"
#include "Tracked.h"
#include "TutorialConfig.h"

#include "lessons/L01_Cpp11_RvalueReferences.h"
#include "lessons/L02_Cpp11_MoveOperations.h"
#include "lessons/L03_Cpp11_StdMove.h"
#include "lessons/L04_Cpp11_PerfectForwarding.h"
#include "lessons/L05_Cpp11_MoveAwareLibrary.h"
#include "lessons/L06_Cpp14_MoveImprovements.h"
#include "lessons/L07_Cpp17_GuaranteedElision.h"
#include "lessons/L08_Cpp20_MoveImprovements.h"
#include "lessons/L09_Cpp23_MoveImprovements.h"
#include "lessons/L10_Pitfalls.h"
#include "lessons/L11_Capstone.h"

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
              << "  not compiled: needs C++" << std_ << ", compiler is C++" << TUTORIAL_COMPILER_STD
              << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    int only = 0;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-t") == 0)
            tut::Tracked::trace(true);
        else
            only = std::atoi(argv[i]);
    }
    (void)&skipped;   // silence "unused function" when every lesson is compiled

    std::cout << "#############################################################\n"
              << "#  move semantics tutorial\n"
              << "#  Compiler standard : C++" << TUTORIAL_COMPILER_STD
              << "   (__cplusplus / _MSVC_LANG = " << TUTORIAL_CPLUSPLUS << "L)\n"
              << "#  Lessons compiled  : C++11 ... C++" << TUTORIAL_COMPILER_STD << '\n'
              << "#############################################################\n";

#if TUTORIAL_INCLUDE(11)
    run_lesson(1, only, &l01_cpp11_rvalue_references::run);
    run_lesson(2, only, &l02_cpp11_move_operations::run);
    run_lesson(3, only, &l03_cpp11_std_move::run);
    run_lesson(4, only, &l04_cpp11_perfect_forwarding::run);
    run_lesson(5, only, &l05_cpp11_move_aware_library::run);
#endif

#if TUTORIAL_INCLUDE(14)
    run_lesson(6, only, &l06_cpp14_move_improvements::run);
#else
    skipped(6, 14);
#endif

#if TUTORIAL_INCLUDE(17)
    run_lesson(7, only, &l07_cpp17_guaranteed_elision::run);
#else
    skipped(7, 17);
#endif

#if TUTORIAL_INCLUDE(20)
    run_lesson(8, only, &l08_cpp20_move_improvements::run);
#else
    skipped(8, 20);
#endif

#if TUTORIAL_INCLUDE(23)
    run_lesson(9, only, &l09_cpp23_move_improvements::run);
#else
    skipped(9, 23);
#endif

#if TUTORIAL_INCLUDE(11)
    run_lesson(10, only, &l10_pitfalls::run);
#endif

#if TUTORIAL_INCLUDE(17)
    run_lesson(11, only, &l11_capstone::run);
#else
    skipped(11, 17);
#endif

    std::cout << "\n#############################################################\n"
              << "#  Lessons run: " << g_lessonsRun << "   not compiled: " << g_lessonsSkipped
              << "   (compiler C++" << TUTORIAL_COMPILER_STD << ")\n"
              << "#############################################################\n";
    return 0;
}
