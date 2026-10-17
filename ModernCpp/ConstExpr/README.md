# ConstExpr: constexpr / consteval from C++11 to C++23

Each lesson in `lessons/` is one chapter. Read it from top to bottom, then run it.
Every `static_assert` is a proof: if the project builds, the compiler computed that value.

| Lesson | Standard | Topic |
|---|---|---|
| L01 | C++11 | constexpr variables, single-return constexpr functions |
| L02 | C++11 | literal types, constexpr constructors, UDLs, static constexpr members |
| L03 | C++14 | relaxed constexpr: locals, loops, mutation, variable templates |
| L04 | C++17 | `if constexpr` |
| L05 | C++17 | constexpr lambdas, inline variables, constexpr library |
| L06 | C++20 | `consteval` immediate functions, compile-time validated values |
| L07 | C++20 | `constinit` |
| L08 | C++20 | `std::is_constant_evaluated()` |
| L09 | C++20 | new/delete, `std::vector`, `std::string` at compile time |
| L10 | C++20 | virtual calls, try/catch, unions, class-type template parameters |
| L11 | C++23 | `if consteval`, consteval propagation |
| L12 | C++23 | static constexpr locals, relaxed rules, constexpr `unique_ptr`/`bitset`/`to_chars` |
| L13 | C++20 | pitfalls, compiler limits, debugging tips |
| L14 | C++20 | capstone: CRC-32 table, string switch, 88 piano keys |

## Choosing the lessons: the compiler standard

Project Properties > C/C++ > Language > C++ Language Standard decides which
lessons are compiled:

| Compiler setting | Lessons compiled |
|---|---|
| C++14 | L01 - L03 |
| C++17 | L01 - L05 |
| C++20 | L01 - L10, L13, L14 |
| C++23 (`/std:c++latest`) | L01 - L14 |

Some C++23 features aren't in every compiler yet. Those parts check a
feature-test macro and print "not supported" instead of failing to build.

The console banner shows the standard in use, and every line of output is
tagged with the standard that introduced the feature, e.g. `[C++14]`.

Visual Studio has no C++11 setting (its oldest is C++14). To compile as C++11,
use GCC or Clang, in WSL or on godbolt.org:

    g++ -std=c++11 ConstExpr.cpp

## Forced errors: `TUTORIAL_FORCE_ERRORS` in `TutorialConfig.h`

| Value | Effect |
|---|---|
| `0` | build and run normally (default) |
| `1` | also compile the **ERROR DEMO** snippets |

Each demo sits next to the code it is about. It is labelled `[L01-E1]`,
`[L01-E2]` and so on, so you can match each Error List entry to its code. It
also says which standards it fails in:

- `fails in: every standard`: a common constexpr mistake, always an error.
- `fails in: C++11 only` (or a similar range): an error under that compiler
  setting, but it compiles cleanly under a newer one. Build with two compiler
  settings to see the rule change.

With `TUTORIAL_FORCE_ERRORS 1` and C++23 you get about 60-70 intentional
errors, depending on the compiler.

## Building with CMake and Clang (and VS Code)

`CMakeLists.txt` and `CMakePresets.json` build the same code with Clang.
The Visual Studio project is unchanged, so you can use either.

Each preset picks the compiler and the C++ standard. The standard decides
which lessons compile, just like the Visual Studio setting.

| Preset | Where | Standards |
|---|---|---|
| `windows-clang-cpp14` ... `windows-clang-cpp23` | Windows: clang-cl with Microsoft's standard library | C++14 - C++23 |
| `clang-cpp11` ... `clang-cpp23` | WSL, Linux, macOS: clang++ | C++11 - C++23 |

C++11 needs a non-Windows preset: Microsoft's standard library requires C++14.

**Windows setup (once).** In the Visual Studio Installer, under
*Desktop development with C++*, tick:
- **C++ Clang Compiler for Windows**
- **C++ CMake tools for Windows** (provides CMake and Ninja)

**In VS Code**
1. Install the recommended extensions: *CMake Tools* and *C/C++*. VS Code
   offers them when you open the folder, because of `.vscode/extensions.json`.
2. Open the `ConstExpr` folder.
3. In the status bar, pick a configure preset, e.g. *Clang (Windows) C++20*.
4. Use **Build**, then **Run** or **Debug** (the play and bug icons in the status bar).
5. To try another standard, pick another preset. Each one builds in its own
   `build/<preset>` folder.

**In a terminal** (on Windows: a *Developer PowerShell for VS*)

    cmake --preset windows-clang-cpp20
    cmake --build --preset windows-clang-cpp20
    build\windows-clang-cpp20\ConstExpr.exe

Forced errors work the same way: set `TUTORIAL_FORCE_ERRORS 1` in
`TutorialConfig.h` and build. The CMake build tells Clang to report every
error, not only the first 20.

## Running

    ConstExpr.exe       all compiled lessons
    ConstExpr.exe 4     only lesson L04
