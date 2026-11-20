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

## The concept and how it evolved

`constexpr` lets the compiler compute values while it compiles the program.
Each standard widened what can run at compile time: from one-line formulas in
C++11 to containers, virtual calls and `std::unique_ptr` in C++23.

**C++11: the strict start (L01, L02)**
- A `constexpr` variable's value is known at compile time (and it is const).
  A plain `const` variable may hold a runtime value; only `const` integers get
  the old C++98 "usable at compile time" rule.
- A `constexpr` function *may* run at compile time when its arguments are
  constants; otherwise it runs normally.
- The body must be a single `return`: `?:` for branches, recursion for loops.
- Your own classes work at compile time if they are *literal types*: a
  `constexpr` constructor with an empty body, a trivial destructor and literal
  members. `constexpr` member functions are implicitly `const`, so nothing can
  be modified.

**C++14: functions that look like normal code (L03)**
- Local variables, loops, `if`/`switch` and several `return`s are allowed.
- Objects created during the evaluation may be modified; `constexpr` member
  functions are no longer implicitly `const`.
- Variable templates: `template <class T> constexpr T pi = ...;`
- Still not allowed: uninitialised locals, `static` locals, `goto`, `try`,
  virtual calls, `new`/`delete`.

**C++17: compile-time branching (L04, L05)**
- `if constexpr` picks a branch at compile time. Inside a template the other
  branch is discarded and never instantiated, which replaces most tag dispatch
  and SFINAE.
- Lambdas are `constexpr` automatically when they can be.
- `inline constexpr` gives one object for the whole program, and `static
  constexpr` members no longer need an out-of-class definition.
- Much more of the library is `constexpr`: `std::array`, `std::string_view`,
  `std::clamp`, ...

**C++20: the big expansion (L06 - L10)**
- `consteval`: an *immediate* function that **must** run at compile time;
  every call has to be a constant.
- `constinit`: initialised at compile time but changeable at runtime, which
  avoids the static initialisation order problem.
- `std::is_constant_evaluated()`: one function, separate compile-time and
  runtime paths.
- Transient allocation: `new`/`delete`, `std::vector`, `std::string` and most
  algorithms work at compile time, as long as all memory is freed before the
  evaluation ends.
- Also allowed: virtual calls, `dynamic_cast`, `try`/`catch` blocks (but no
  throwing), changing a union's active member, uninitialised locals, and class
  types or floating-point values as template parameters.

**C++23: closing the gaps (L11, L12)**
- `if consteval { } else { }` replaces `is_constant_evaluated()`, and its
  compile-time branch may call `consteval` functions with the function's own
  parameters.
- Consteval propagation: a template or lambda that calls a `consteval`
  function with its parameter becomes `consteval` itself.
- `static constexpr` locals inside `constexpr` functions; `static` variables
  and `goto` may appear as long as evaluation never reaches them.
- More library: `constexpr` `std::unique_ptr`, `std::bitset`, integer
  `std::to_chars`, and fully `constexpr` `std::optional` / `std::variant`.

**Pitfalls (L13)**
- `constexpr` on a function means *may*, not *must*. Force compile-time
  evaluation with a `constexpr` variable, `static_assert`, a template argument
  or `consteval`.
- Undefined behaviour (overflow, out-of-bounds access) is a compile error
  during constant evaluation, so `static_assert` tests catch it.
- Compilers limit recursion depth and evaluation steps, and floating-point
  results still need a tolerance.

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
