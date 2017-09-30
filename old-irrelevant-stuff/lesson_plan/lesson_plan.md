Template metaprogramming for fun and profit
===========================================
By Gašper Ažman

Foreword and Acknowledgements
=============================

These are the lecture notes and the lecture preparation text I write for myself
before the lecture. I'll try to expand them to a full set of lecture notes as
the lecture series goes along.

I have to thank the following people for contributions to the lecture (so far!):

- Alex Stepanov: for teaching me how to think, inspiring me to actually do a
  lecture series, and writing _Elements of Programming_, a book without which my
  thoughts about expression would be much more entangled than they are now.
- Dmitry Lapanik: for many a discussion, errata on CRTP, and all the code
  contributions to the library.
- Jeffrey Li: for asking all the right questions, being a sounding board, and
  making sure I get stuff right.
- Peter Cecil: for braving the lectures as a complete newbie to c++ and telling
  me everything he does not understand. Content calibration would have been
  nigh-impossible without him.
- Rahul Dutta: for being a Manager (caps deserved) who understands the value of
  spreading knowledge and constantly pushing me to do more of it.
- Wei Chen: for being a sounding board, testing the build system, and just
  generally being a really valuable nuisance in finding bugs.
- William Tyler: for the idea about presenting _Expression Templates_ by
  implementing comparison chaining a-la math and python, asking questions and
  feedback.
- Scott Wu and the rest of the IT team at A9: for steadfastly recording, editing
  and uploading the lecture videos.

Lesson 1: Substitutability
==========================

Bringing Statically-Checked Duck-Typing to C++ without sacrificing performance
------------------------------------------------------------------------------
_(Teaser for the rest of the course)_

Code reuse results from the same piece of code working with many others.

Duck-typing comes from "If it quacks like a duck, it's a duck." The point is
that if we have an algorithm that requires a certain interface on its inputs, it
will run with any object that provides that interface. This is called
substitutability.

In some languages, we get this out of the box. Python, and lisp, perl etc. are
such languages. These languages use name-based dispatch at runtime, and incur
the performance costs. They also can't type-check your program at compile time,
making "passing string instead of int" a run-time error.

In C++, we get this out of the box at compile-time with templates, because
template parameters are duck-typed. We don't however, get it at run-time.

If you've done object-oriented programming in C++, you saw you have to declare
some methods virtual, and then you have to inherit from some class that
specifies the virtual table that we call the abstract base class, and then you
can have pointers to the abstract base class that will actually dispatch to the
derived class.

### The definition of substitutability
Let's examine the problem inheritance _really_ solves: The definition of
*substitutability*, at compile time.

By inheriting, you're really promising that you support a certain interface.

Let's take a look at `to_stringable`.

    :::c++
    struct to_stringable {
      virtual std::string to_string() const = 0;
    };

The implementation class looks like this:

    :::c++
    struct my_class : to_stringable {
      std::string to_string() const {
        return "I can stringify myself!";
      }
    }

Problems:

- every time we pass `my_class` by `const&`, for instance, all calls to
  `to_string()` will be virtual, because the compiler can't prove that the
  reference isn't to some derived class of `my_class`.
- it couples the type definition with some arbitrary system requirements that
  the type _sometimes_ participate in virtual dispatch. You have to declare that
  with the type definition, you can't just add it to types you get from a
  library.
- multiple inheritance, which you would need if you wanted to inherit from every
  interface your program might need, inflates the size of your objects. Always,
  even if you don't end up needing that interface.
- It violates separation of concerns: your type definition shouldn't know about
  what arbitrary things you want it to support virtually in the rest of the
  system. It's wrong coupling.

Solution: _type erasure_.

Let's take a look at what the definition of the above would look with the
library we want to write:

    :::c++
    template <typename C>
    struct to_stringable_concept_t : C {
      virtual std::string to_string() const = 0;
    };

    template <typename M>
    struct to_stringable_model_t : M {
      std::string to_string() const {
        return M::self().value().to_string();
      }
    };

    template <typename I>
    struct to_stringable_interface_t : I {
      std::string to_string() const {
        namespace f = type_erasure::feature_support;
        return f::ifc_concept_ptr(*this)->to_string();
      }
    };

    struct to_stringable {
      template <typename I>
      using interface = to_stringable_interface_t<I>;

      template <typename M>
      using model = to_stringable_model_t<M>;

      template <typename C>
      using concept = to_stringable_concept_t<C>;
    };

So, that's a fair bit of overhead, to be sure, compared with just what is
basically the `concept_t` part. However, it's pretty easily written, and you can
copy and paste it when writing new features. The true win comes when you take a
look at the definition of `my_class`:

    :::c++
    struct my_class {
      std::string to_string() const {
        return "I can stringify myself!";
      }
    };

Nothing virtual. `my_class` doesn't even have to know it will be participating
in virtual dispatch at all. This means you get to choose whether to incur the
virtual method call overhead, you get to use external library types, and most
importantly, it's now a regular type.

This is how you use it:

    :::c++
    int main() {
      using namespace type_erasure;
      using namespace type_erasure::features;
      any<to_stringable, regular> x = my_class();
      std::cout << x.to_string() << "\n";
    }


Lesson 2: Types and composition (simple, product and sum-types)
===============================================================
Note: definitions are taken from Stepanov, McJones: Elements of Programming.

Types
-----
A _value type_ is the correspondence between a _representation_, a series of
bits, and its _interpretation_.

We have two kinds of simple types: _primitive_ types, like `int`, `unsigned
int`, `char`, `bool` and the like. We also have so-called 'atoms'. In C++, atoms
are modelled with empty `struct`s. An atom is a type which has only one possible
value, and is only ever equal to itself.

Out of those, we build more complex types. How? By *composition*.

There are two principal ways we can compose types (thus getting aggregate types)

- we can make a sequence of them: these are called _product types_. There are
  two ways we can go about this:
    - a sequence of only one type (homogeneous sequence) - arrays, lists ...
    - a fixed-length sequence of different (heterogeneous sequence) - `tuple`s,
      `struct`s
- we can say "this type can be any one of these types": so-called _sum types_.
  Again, this can be done in (roughly) two ways:
    - _variant_s: require us to list all their possible sub-types up-front. They
      act as containers for them, and we have to figure out the type they hold
      before we do anything with it. Unions (and boost::variant) are of this kind.
      I will also call these _boxed_ sum-types, because they require us to take
      the value out of the box (discover the type) before we can use it. These
      don't impose requiremends on the values they can hold.
    - _interfaces_: don't require us to list all their possible sub-types ahead of
      time. We also don't have to figure out the actual type they hold before we
      use them. Instead, the type of the interface specifies what can be done with
      any one of its sub-types, and we can press buttons of the interface and it
      will do the right thing with the value automagically. Inheritance models
      this kind of sum type.

Let's look at the properties of composition that types that we got from these
two properties offer us.

The requirements for composition
--------------------------------

What operations a "composable" type needs to support is strongly dependent on
what one usually requires of an arbitrary type.

For instance, if you want to copy an aggregate type, all of its sub-types must
be copyable.

If you want to move an aggregate type, all of its sub-types must be movable
(copyable implies movable).

Should you want to default-construct your type, all of its sub-types need to be
default-constructible. Same for move- and copy- constructibility.

If you want to compare an aggregate type for equality... Well, it seems to be
almost a requirement for all of the sub-types to be equality-comparable.
However, this is a bit of a long-shot. Equality comparison involves two objects,
and this requires special treatment. We'll return to this in a minute.

Some types, in particular, those representing physical resources, cannot be
copied. Those, however, have absolutely no excuse not to be movable. The
majority of types, though, copyability is possible, and should therefore be
provided.

Types holding resources, by virtue of their movability, can almost always be
made default constructible. Value types are not such. Some value types have
sensible default values. Int has this property, for instance. A type that
pretends to be a reference, cannot.

### C++: What you get by default

The above conundrums get us to the following set of default capabilities in c++.

Let's take a look at an atom:

    :::c++
    struct foo {};

Which things just automatically work for foo?

    :::c++
    {                     // start a scope
    foo x;                // default construction - works
    auto y(x);            // copy-construction - works
    auto z(std::move(x)); // move construction - works
    y = x;                // copy-assignment - works
    z = std::move(y);     // move construction - works
    x == y;               // does not work
    }                     // destruction at end of scope - works

So, the above definition of foo is basically the same as if we wrote the
following:

    :::c++
    struct foo {
      foo() = default;                      // default constructor
      foo(foo const&) = default;            // copy constructor
      foo(foo&&) = default;                 // move constructor
      foo& operator=(foo const&) = default; // copy assignment
      foo& operator=(foo&&) = default;      // move assignment
      ~foo() = default;                     // destructor
    };

All of this, by default, you get for free, with no code.

The minute you choose to provide one constructor, though:

    :::c++
    struct foo {
      foo(foo const&) = default;
    };

... the default constructor gets deleted.

Basically, the rules are a bit complicated, but that's because they try to do
the sensible thing in all situations.

Also, mind that the rules speak about *declaring*, not defining. If you mention
it in a class, you've declared it, even if you declare it default or deleted.

* _default constructor_: you get it unless you declare any other constructor.
* _copy constructor_: you get it if you don't declare your own, and if all of the
  members have one.
* _move constructor_: you don't declare a copy constructor, no copy assignment
  operator, no move assignment operator and no user-declared destructor, not
  deleted, and all the members and bases are moveable. Basically, if you
  breathe, you don't get it.
* _copy assignment operator_: you don't declare one and the compiler knows how to
  make one.
* _move assignment operator_: same as for move constructor, except you don't get
  one if a move constructor is declared.
* _destructor_: you get it unless you declare your own.

Product types:
--------------

What do the methods provided by default mentioned above give you for product
types? They do the sensible thing - they repeat the operation on all the
members. Default construction? Default-constructs all the members. Destruction?
Calls the destructor of all the members. Copy construction and assignment?
Copy member-by-member. Move construction and assignment. Ditto.

From the rules above, it's pretty clear that the language just does the right
thing for product types if you don't write anything. If one of the members is
move-only? Well, you don't get the copy constructor. All the members are
copyable? You get a copy constructor. One of the members is not default
constructible? You don't get a default constructor.

The fun starts when you write one or more of the methods yourself. If you do,
you better explicitly specify what you want for all the others, because things
will get strange otherwise.

For product types, even other operations are relatively simple. Most of the
time, if the members have an equality comparison operator declared, the correct
way to define is to just compare members pair-by-pair.

    :::c++
    struct banana {
      banana_peel peel;
      banana_core core;
      friend operator==(banana const& x, banana const& y) {
        return x.peel == y.peel && x.core == y.core;
      }
      friend operator!=(banana const& x, banana const& y) {
        return !(x == y);
      }
    };

But what of pointers?  There, of course, we should compare pointees! But only if
both are non-null, of course. The point is that it gets messy, and the committee
only likes defining things that are really obviously right. Sometimes you really
*do* want to just compare the pointer values, for instance (such as with
interned string types).

    :::c++
    struct baloon_thread {
      baloon * x;
      friend operator==(baloon_thread const& x, baloon_thread const& y) {
        return x.x && y.x && *x.x == *y.x;
      }
      friend operator!=(baloon_thread const& x, baloon_thread const& y) {
        return !(x == y);
      }
    };

So, for this and other reasons, the committee decided to not autogenerate
equality operators. It's a shame, but please do it - your future self and your
colleagues will thank you for it.

Sum Types
---------

Sum types are types whose value spaces are sums of subtypes. This happens when a
type can represent a value of either one or another type, but not at the same
time.

There are two kinds of sum types: the closed and open kind.

The closed kind is exemplified by discriminated unions:

    struct either_int_or_char_array {
      enum which_type {
        INT,
        STRING
      } type;
      union {
        int integer;
        std::string str; // EERRRRR doesn't work, not a POD.
      } member;
    };

So, if you want to know what a sort of a minimal sum-type implementation for a
type that *does* have a nontrivial constructor looks like, this is it:

    :::include ../example_code/hand_rolled_union.cpp UNION

TODO: write about how inheritance, multiple inheritance, vtables, virtual
inheritance, and object layout work.


Lesson 3: Type inference, operators and mixins 
==============================================
_and the Curiously Recurring Template Pattern (CRTP)_.

Type inference
--------------

Let's take a look at what types look like in C++.

Simplest example:

    :::c++
    int i = 0;

Here, i becomes a mutable variable of type `int`. We'll call `int` the _value
type_.

Let's add specifiers. We have two:

    :::c++
    int const i = 0;
    int volatile j = 0;
    int const volatile k = 0;

The `const` specifier means _we_ won't change the variable. In this particular
case, since the variable was defined as `const`, not just declared `const` in the
current context, it is actually undefined behavior to change it.

The volatile specifier means that the compiler can't reason about the value of
the variable, but must treat it as a pure memory location. This is useful when
you know that the memory location can change without the compiler knowing about
it - such as from a different thread, or from direct memory IO. It is not often
useful, and if you need it, you probably know you do.


Lecture 4: Fucking Templates, How Do They Work
----------------------------------------------

Templates are a purely functional language embedded in C++. Let's figure out how
that language is structured.

Let's start from the ground up. What are the values of this language?
- integer constants
- the value type:

    :::c++
    template <typename T, T V>
    struct integral_constant {
      static T constexpr value = V;
      using value_type = T;
      constexpr operator value_type() const { return value; }
    };

- the 'type' type:

    :::c++
    template <typename T>
    struct type_ {
      using type = T;
    };

- lists:

    :::c++
    template <typename... Ts>
    struct list {};

- list functions:
    - concacenate two:

    :::c++
    template <typename List1, typename List2>
    struct concatenate_two;

    template <typename... Ts, typename Us...>
    struct concatenate_two<list<Ts...>, list<Us...>> {
      using type = list<Ts..., Us...>;
    };

    template <typename List1, typename List2>
    using concatenate_two_t = typename concatenate_two<List1, List2>::type;


    - concatenate many:

    :::c++
    template <typename... Lists>
    struct concatenate;

    template <>
    struct concatenate<> {
      using type = list<>;
    };

    template <typename... Ts>
    struct concatenate<list<Ts...>> {
      using type = list<Ts...>;
    };

    template <typename... Ts, typename... Us, typename... Lists>
    struct concatenate<list<Ts...>, list<Us...>, Lists...> {
      using type = typename concatenate<list<Ts..., Us...>, Lists>::type;
    }

    template <typename... Lists>
    using concatenate_t = typename concatenate<Lists...>::type;


    - map:

    


Appendix A: Topics
------------------

_(not in order)_

- The Basics:
    - Type inference:
        - Auto and decltype
        - Return type deduction
    - Lambdas, generic lambdas, closures and std::function.
    - Move semantics (and all that stuff with rvalue references, such as perfect
      forwarding)
    - Templates (and why you should care)
    - Template type aliases
    - Template variables
    - Type functions
    - Variadic templates
    - Regular types (and why they matter)
    - Pure functions (and why they are suddenly fast)
    - The `constexpr` keyword and compile-time computation of values
    - How to help the compiler check your code for bugs (encoding knowlege in
      types)
- The advanced stuff (idioms and more):
    - Compile-time computation (of both values and types):
    - Template metaprogramming, the new and easy way (the way of boost::hana)
    - Template metaprogramming, the functional way
    - How to do mixin classes in C++ without virtual functions (CRTP)
    - Type erasure (how to never need inheritance to do polymorphism again,
      courtesy of inheritance)
    - Making a class property in C++ (an expansion of my CppCon lightning-talk, a
      proper property)
    - Smart pointers and when you should use them (Working title: don’t use
      shared_ptr)
    - Inversion of control (Working title: death to config objects)
    - Small buffer optimization, placement new, and a recap of Alexandrescu’s talk
      "Allocators are to Allocation what Vector is to Vexation” from CppCon this
      year


### Requirements tree for ordering topics

Start at the end and weave requirements:

- A proper property
    - object layout
        - offsetof
    - templated implicit conversion operators
        - implicit conversion operators
            - operator overloading
    - type tags
    - complete and incomplete types
    - compile-time function dispatch
    - reinterpret cast
    - the const cast trick
        - const cast
    - macros

- Type erasure: writing std::function.
    - polymorphism through inheritance
        - virtual method dispatch
    - `unique_ptr`






