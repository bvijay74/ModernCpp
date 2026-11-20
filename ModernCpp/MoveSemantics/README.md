# MoveSemantics: rvalue references, move semantics and perfect forwarding, C++11 to C++23

Each lesson in `lessons/` is one chapter. Read it from top to bottom, then run it.
`Tracked.h` defines a type that counts every copy and move, so the output shows
what each line of code really did, e.g.

    [C++11]    push_back(Tracked("pushed")) ............. 0 copies, 1 move
    [C++11]    emplace_back("emplaced") ................. 0 copies, 0 moves

| Lesson | Standard | Topic |
|---|---|---|
| L01 | C++11 | value categories, rvalue references, binding rules, ref-qualified members |
| L02 | C++11 | move constructor and assignment, rule of five / zero, when moves are *not* generated |
| L03 | C++11 | `std::move` is a cast, sink parameters, moved-from state, `noexcept` and `vector` growth |
| L04 | C++11 | perfect forwarding: forwarding references, reference collapsing, `std::forward`, `emplace_back` |
| L05 | C++11 | the move-aware library: `unique_ptr`, `swap`, move iterators, moving containers |
| L06 | C++14 | init-capture `[p = std::move(p)]`, `std::exchange`, `make_unique`, `decltype(auto)` |
| L07 | C++17 | guaranteed copy elision, non-movable factories, map node handles, `try_emplace` |
| L08 | C++20 | more implicit moves, pack init-capture, concepts (`std::movable`) |
| L09 | C++23 | deducing `this`, `std::forward_like`, `std::move_only_function`, `auto(x)` |
| L10 | C++11 | pitfalls: use after move, forwarding twice, constructor hijacking, `return std::move` |
| L11 | C++17 | capstone: an audio pipeline - copy style vs move style |

## The concept and how it evolved

Move semantics let an object hand over its resources (heap memory, a file, a
socket) instead of copying them. C++11 built the mechanism; every later
standard removed copies and moves that the compiler can see aren't needed.

**C++11: the foundation (L01 - L05)**
- Value categories: an *lvalue* has an identity (a variable), a *prvalue* is a
  temporary, and an *xvalue* is an expiring object such as `std::move(x)`.
- The rvalue reference `T&&` binds only to rvalues, so a function taking
  `T&&` may steal from its argument. A *named* rvalue reference is an lvalue.
- The move constructor and move assignment steal the resource and leave the
  source empty but valid. Rule of five; better, rule of zero. A user-declared
  destructor or copy operation stops the compiler from generating moves, so
  "moves" quietly become copies.
- `std::move` moves nothing: it is a cast to an rvalue. On a `const` object it
  gives `const T&&`, which copies. `std::vector` only moves its elements when
  it grows if the move constructor is `noexcept`.
- Perfect forwarding: a *forwarding reference* (`T&&` with a deduced `T`, or
  `auto&&`), reference collapsing and `std::forward<T>` pass an argument on
  with its original value category. `std::vector<T>&&` and `const T&&` are not
  forwarding references.
- The library: move-only `std::unique_ptr`, O(1) container moves, `std::swap`
  by moves, the `std::move` algorithm, move iterators and `emplace_back`.

**C++14: filling the gaps (L06)**
- Init-capture `[p = std::move(ptr)]` moves objects into lambdas.
- `std::make_unique`, `std::exchange` for tidy move operations,
  `decltype(auto)` to forward return values, and generic lambdas with
  `std::forward<decltype(x)>(x)`.

**C++17: fewer moves (L07)**
- Guaranteed copy elision: a prvalue initialises its destination directly, so
  returning `T(args)` needs no copy or move constructor at all. Returning a
  *named* local (NRVO) is still optional.
- Map/set node handles move elements between containers without touching
  them; `try_emplace` doesn't steal from its arguments when the key exists.

**C++20: more implicit moves (L08)**
- `return x;` also moves when `x` is an rvalue-reference parameter.
- A whole parameter pack can be moved into a lambda:
  `[...args = std::forward<Args>(args)]`.
- Concepts (`std::movable`, `std::copyable`) and `requires` clauses constrain
  forwarding templates, with short error messages.

**C++23: forwarding the object itself (L09)**
- Simpler implicit move: a returned name is an rvalue even when the function
  returns `T&&`.
- Deducing `this` (`this Self&& self`): one member function forwards the
  object itself, replacing `&` / `const&` / `&&` overloads.
  `std::forward_like` forwards a member with its owner's value category.
- `std::move_only_function` stores move-only callables, and `auto(x)` makes an
  explicit copy.

**Pitfalls (L10)**
- Using an object after moving from it, or forwarding the same argument twice.
- A forwarding constructor can hijack the copy constructor; constrain it.
- `return std::move(local);` prevents elision and costs a move.
- `std::move` through a `const` reference copies, and `std::move` doesn't
  extend a temporary's lifetime.

## Choosing the lessons: the compiler standard

Project Properties > C/C++ > Language > C++ Language Standard decides which
lessons are compiled:

| Compiler setting | Lessons compiled |
|---|---|
| C++14 | L01 - L06, L10 |
| C++17 | L01 - L07, L10, L11 |
| C++20 | L01 - L08, L10, L11 |
| C++23 (`/std:c++latest`) | L01 - L11 |

Some C++23 features aren't in every compiler yet. Those parts of L09 check a
feature-test macro and print "skipped" instead of failing to build.

Visual Studio has no C++11 setting (its oldest is C++14). To compile as C++11,
use GCC or Clang, in WSL or on godbolt.org:

    g++ -std=c++11 MoveSemantics.cpp

## Forced errors: `TUTORIAL_FORCE_ERRORS` in `TutorialConfig.h`

| Value | Effect |
|---|---|
| `0` | build and run normally (default) |
| `1` | also compile the **ERROR DEMO** snippets |

Each demo sits next to the code it is about. It is labelled `[L01-E1]`,
`[L01-E2]` and so on, and says which standards it fails in:

- `fails in: every standard`: a common mistake, always an error.
- `fails in: C++11, C++14` (or a similar range): an error under an older
  compiler setting that compiles cleanly under a newer one. For example
  `[L02-E2]` (returning a non-movable type) fails in C++14 and compiles in
  C++17, which is guaranteed copy elision at work.

## Building with CMake and Clang (and VS Code)

`CMakeLists.txt` and `CMakePresets.json` build the same code with Clang.
The Visual Studio project is unchanged, so you can use either.

| Preset | Where | Standards |
|---|---|---|
| `windows-clang-cpp14` ... `windows-clang-cpp23` | Windows: clang-cl with Microsoft's standard library | C++14 - C++23 |
| `clang-cpp11` ... `clang-cpp23` | WSL, Linux, macOS: clang++ | C++11 - C++23 |

**Windows setup (once).** In the Visual Studio Installer, under
*Desktop development with C++*, tick **C++ Clang Compiler for Windows** and
**C++ CMake tools for Windows**.

**In VS Code**
1. Install the extensions *CMake Tools* and *C/C++*.
2. Open the `MoveSemantics` folder.
3. Click the **CMake** icon in the Activity Bar. Under **Project Status**,
   pick a configure preset, e.g. *Clang (Windows) C++20*.
4. Use the **Build**, **Debug** and **Launch** buttons there, or the
   Command Palette: *CMake: Build*, *CMake: Debug*, *CMake: Run Without Debugging*.
   (The editor's own Run button and F5 compile a single file; they don't use
   the preset.)
5. Optional - to use F5, create `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "MoveSemantics (CMake preset)",
      "type": "cppvsdbg",
      "request": "launch",
      "program": "${command:cmake.launchTargetPath}",
      "args": [],
      "cwd": "${workspaceFolder}",
      "console": "integratedTerminal"
    }
  ]
}
```

**In a terminal** (on Windows: a *Developer PowerShell for VS*)

    cmake --preset windows-clang-cpp20
    cmake --build --preset windows-clang-cpp20
    build\windows-clang-cpp20\MoveSemantics.exe

## Running

    MoveSemantics.exe          all compiled lessons
    MoveSemantics.exe 4        only lesson L04
    MoveSemantics.exe 4 -t     ...and print every copy and move as it happens
