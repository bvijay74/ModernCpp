# =============================================================================
# CMakeLists.txt  --  build the ConstExpr tutorial with Clang (or GCC, MSVC)
# =============================================================================
# The easiest way to use this is through CMakePresets.json:
#   VS Code:  open this folder, pick a configure preset (e.g. "Clang C++20")
#             in the CMake Tools status bar, then Build / Run.
#   Terminal: cmake --preset clang-cpp20
#             cmake --build --preset clang-cpp20
#             ./build/clang-cpp20/ConstExpr
#
# The C++ standard comes from CONSTEXPR_CXX_STANDARD (each preset sets it).
# It decides which lessons compile, exactly like the "C++ Language Standard"
# setting in the Visual Studio project.
#
# Forced errors stay where they always were: TUTORIAL_FORCE_ERRORS in
# TutorialConfig.h.
# =============================================================================
cmake_minimum_required(VERSION 3.21)

project(ConstExpr LANGUAGES CXX)

set(CONSTEXPR_CXX_STANDARD 20 CACHE STRING "C++ standard for the tutorial: 11, 14, 17, 20 or 23")
set_property(CACHE CONSTEXPR_CXX_STANDARD PROPERTY STRINGS 11 14 17 20 23)

# Microsoft's standard library (used by MSVC and by Clang on Windows) needs at
# least C++14, so a C++11 build there silently becomes C++14. Say so.
if(CONSTEXPR_CXX_STANDARD EQUAL 11 AND (MSVC OR WIN32))
    message(WARNING
        "C++11 is not available with Microsoft's standard library; the build will use C++14. "
        "For C++11, build in WSL or on Linux/macOS (preset clang-cpp11).")
endif()

file(GLOB LESSON_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/lessons/*.h")

add_executable(ConstExpr
    ConstExpr.cpp
    Lesson.h
    TutorialConfig.h
    ${LESSON_HEADERS}
)

set_target_properties(ConstExpr PROPERTIES
    CXX_STANDARD ${CONSTEXPR_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF           # -std=c++20, not -std=gnu++20
)

# Warnings, and "show every error": with TUTORIAL_FORCE_ERRORS 1 the tutorial
# produces dozens of intentional errors, but Clang stops after 20 by default.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")      # clang-cl
        target_compile_options(ConstExpr PRIVATE /W4 /clang:-ferror-limit=0)
    else()                                                       # clang++
        target_compile_options(ConstExpr PRIVATE -Wall -Wextra -ferror-limit=0)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ConstExpr PRIVATE -Wall -Wextra)
elseif(MSVC)
    target_compile_options(ConstExpr PRIVATE /W3 /permissive-)
endif()

message(STATUS "ConstExpr tutorial: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}, C++${CONSTEXPR_CXX_STANDARD}")
