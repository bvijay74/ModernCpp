// =============================================================================
// L01 [C++11]  value categories and rvalue references
// =============================================================================
// Every expression has a TYPE and a VALUE CATEGORY:
//   lvalue   has an identity you can refer to again: a variable, *ptr, a[i],
//            a function returning T&. You can take its address.
//   prvalue  a "pure" temporary value: 42, x + 1, a function returning T.
//   xvalue   an "expiring" object: std::move(x), a function returning T&&.
//            It has an identity, but you've said it can be pilfered.
//   (prvalue + xvalue are together called "rvalues".)
//
// C++11 added the RVALUE REFERENCE, written T&&. It only binds to rvalues -
// objects nobody else will look at again - so a function taking T&& may steal
// their contents instead of copying. That is the whole basis of move semantics.
//
//   reference type     binds to
//   T&                 non-const lvalues only
//   const T&           everything (lvalues and rvalues) - the C++98 catch-all
//   T&&                rvalues only
//
// THE rule that surprises everyone: a NAMED rvalue reference is an lvalue.
//   void f(std::string&& s) { ... s ... }   // inside f, `s` is an lvalue
// =============================================================================
#pragma once

#include "../Lesson.h"

#if TUTORIAL_INCLUDE(11)

#include <string>
#include <utility>

namespace l01_cpp11_rvalue_references {

const int STD = 11;

// -----------------------------------------------------------------------------
// 1. Which overload binds? (the binding table in action)
// -----------------------------------------------------------------------------
inline const char* which(int&) { return "int&        (non-const lvalue)"; }
inline const char* which(const int&) { return "const int&  (const lvalue)"; }
inline const char* which(int&&) { return "int&&       (rvalue)"; }

// ERROR DEMO [L01-E1]  fails in: every standard
// A non-const lvalue reference can't bind to a temporary.
#if TUTORIAL_FORCE_ERRORS
void error_demo_lvalue_ref_to_temporary() { int& r = 42; (void)r; }
#endif

// ERROR DEMO [L01-E2]  fails in: every standard
// An rvalue reference can't bind to an lvalue (x has a name).
#if TUTORIAL_FORCE_ERRORS
void error_demo_rvalue_ref_to_lvalue()
{
    int x = 1;
    int&& r = x;
    (void)r;
}
#endif

// -----------------------------------------------------------------------------
// 2. A named rvalue reference is an lvalue
// -----------------------------------------------------------------------------
inline const char* inside(int&& r)
{
    return which(r);   // r has a name -> lvalue -> picks int&
}

inline void take_rvalue(int&&) {}

// ERROR DEMO [L01-E3]  fails in: every standard
// r is declared int&&, but the EXPRESSION `r` is an lvalue, so it can't be
// passed on to another int&& parameter. You need std::move(r) (L03).
#if TUTORIAL_FORCE_ERRORS
void error_demo_pass_on(int&& r) { take_rvalue(r); }
#endif

// -----------------------------------------------------------------------------
// 3. Seeing the value category with decltype
// -----------------------------------------------------------------------------
// decltype((expr)) - note the DOUBLE parentheses - gives
//   T& for an lvalue, T&& for an xvalue, plain T for a prvalue.
template <typename T>
struct category {
    static const char* name() { return "prvalue"; }
};
template <typename T>
struct category<T&> {
    static const char* name() { return "lvalue"; }
};
template <typename T>
struct category<T&&> {
    static const char* name() { return "xvalue"; }
};
#define VALUE_CATEGORY(expr) category<decltype((expr))>::name()

inline std::string make_name() { return "temporary"; }
inline std::string&& expiring(std::string& s) { return std::move(s); }

// -----------------------------------------------------------------------------
// 4. Lifetime extension
// -----------------------------------------------------------------------------
// Binding a temporary to const T& or T&& keeps it alive as long as the
// reference. With T&& you may also modify it.

// -----------------------------------------------------------------------------
// 5. Overloading member functions on the object's value category
// -----------------------------------------------------------------------------
// Ref-qualifiers (C++11): `&` = called on an lvalue object, `&&` = called on an
// rvalue object. A getter can then copy from lvalues but move from temporaries.
class Patch {
public:
    explicit Patch(std::string name) : name_(std::move(name)) {}

    std::string name() const& { return name_; }              // copy: object lives on
    std::string name() && { return std::move(name_); }       // move: object is expiring

    // Only allowed on temporaries: "take the name and throw the patch away".
    std::string release() && { return std::move(name_); }

private:
    std::string name_;
};

// ERROR DEMO [L01-E4]  fails in: every standard
// release() is &&-qualified: it can't be called on the lvalue `p`.
#if TUTORIAL_FORCE_ERRORS
std::string error_demo_release_lvalue()
{
    Patch p("Strings");
    return p.release();
}
#endif

// -----------------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------------
inline void run()
{
    tut::header(STD, "L01", "value categories and rvalue references");

    tut::section(STD, "1. which overload binds?");
    int x = 1;
    const int cx = 2;
    tut::show(STD, "which(x)", which(x));
    tut::show(STD, "which(cx)", which(cx));
    tut::show(STD, "which(42)", which(42));
    tut::show(STD, "which(x + 1)", which(x + 1));
    tut::show(STD, "which(std::move(x))", which(std::move(x)));

    tut::section(STD, "2. a named rvalue reference is an lvalue");
    tut::show(STD, "inside(int&& r) -> which(r)", inside(7));

    tut::section(STD, "3. value categories via decltype((expr))");
    std::string s = "abc";
    tut::show(STD, "x", VALUE_CATEGORY(x));
    tut::show(STD, "42", VALUE_CATEGORY(42));
    tut::show(STD, "x + 1", VALUE_CATEGORY(x + 1));
    tut::show(STD, "s[0]", VALUE_CATEGORY(s[0]));
    tut::show(STD, "\"literal\" (an array lvalue!)", VALUE_CATEGORY("literal"));
    tut::show(STD, "make_name()  (returns T)", VALUE_CATEGORY(make_name()));
    tut::show(STD, "std::move(s)", VALUE_CATEGORY(std::move(s)));
    tut::show(STD, "expiring(s)  (returns T&&)", VALUE_CATEGORY(expiring(s)));

    tut::section(STD, "4. lifetime extension");
    const std::string& viaConst = make_name();   // temporary lives as long as viaConst
    std::string&& viaRvalue = make_name();        // ...and through T&& it is modifiable
    viaRvalue += " (modified)";
    tut::show(STD, "const std::string& r = make_name()", viaConst);
    tut::show(STD, "std::string&& r = make_name(); r += ...", viaRvalue);

    tut::section(STD, "5. ref-qualified member functions");
    Patch piano("Grand Piano");
    tut::show(STD, "piano.name() (lvalue: copies)", piano.name());
    tut::show(STD, "piano still has its name", piano.name());
    tut::show(STD, "Patch(\"Organ\").name() (moves)", Patch("Organ").name());
    tut::show(STD, "std::move(piano).release()", std::move(piano).release());
}

} // namespace l01_cpp11_rvalue_references

#endif // TUTORIAL_INCLUDE(11)
