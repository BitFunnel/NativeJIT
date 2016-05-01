## Hello World for NativeJIT

We're going to create a function, build an expression tree, compile the function, and then run the function.

### Create function

The are three parameters: argument types, allocator, and code buffer. 

#### Argument types

Template type parameters define return and argument types for your function. Can have 0 to 4 arguments.

For example, if you're providing a function that computes the area of a circle, the prototype might be something like:

~~~
float area(float radius);
~~~

This would correspond to a NativeJIT type signature of:

~~~
<float, float>
~~~

In general, the first parameter is the return value and the remaining parmeters are the return types.

#### Allocator

The provides the memory where expression nodes will reside. There are a lot of ways you can do this, but a reasonable default is the allocator in `Temporary` directory.

~~~
ExecutionBuffer codeAllocator(8192);
Allocator allocator(8192);
~~~~

Note that these sizes don't necessarily have to be the same.

#### Code buffer

The code buffer provides executable memory where the compile code will reside.

~~~
FunctionBuffer code(codeAllocator, 8192);
Function(Allocators::IAllocator& allocator, FunctionBuffer& code);
~~~

### Build expression tree

We're going to construct a tree where the interior nodes are operators and the leaf nodes are either literals or function parameters.

Contiuning our radius example,

~~~
Function<float, float> expression(allocator, code);

const float  PI = 3.14159265358979f;
auto & a = expression.Mul(expression.GetP1(), expression.GetP1());
auto & b = expression.Mul(a, expression.Immediate(PI));
~~~

In the code above, `.GetP1()` refers to the first (and in this case, only) parameter.

Note that the tree is built bottom-up, since the children are passed to their parents. Each node has 0 to N inputs and exactly 1 output. These are typed, and anything that's type-compatible is legal.

TODO: add picture.

### Compile

~~~
auto computeRadius = expression.Compile(b);
~~~

This translates your function into `x64` code.

### Run

You call this just like calling a C++ function!

~~~
auto result = computeRadius(radius_input);
~~~

### Putting it all together

~~~
#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"

#include <iostream>

using NativeJIT::Allocator;
using NativeJIT::ExecutionBuffer;
using NativeJIT::Function;
using NativeJIT::FunctionBuffer;

int main()
{
    ExecutionBuffer codeAllocator(8192);
    Allocator allocator(8192);
    FunctionBuffer code(codeAllocator, 8192);

    const float  PI = 3.14159265358979f;

    Function<float, float> expression(allocator, code);

    auto & a = expression.Mul(expression.GetP1(), expression.GetP1());
    auto & b = expression.Mul(a, expression.Immediate(PI));
    auto function = expression.Compile(b);

    float p1 = 2.0;

    auto expected = PI * p1 * p1;
    auto observed = function(p1);

    std::cout << expected << ":" << observed << std::endl;

    return 0;
}
~~~

TODO: explain how to link this.


## Design notes and warnings

It's assumed that everything is side-effect free, including functions that are called from NativeJIT. Calling a function that has side effects from JIT'ed code (e.g., calling `printf`) is undefined from an API standpoint. From an implementation standpoint, the current implementation guarantees that each function will be called at least once. This at least once behavior is explained by the implementation of conditionals, which are hack-y because we haven't needed "real" conditionals in Bing and adding "real" conditionals would require re-writing the register allocator, which is non-trivial.

In particular, register allocation was originally done using [Sethi-Ullman](https://en.wikipedia.org/wiki/Sethi%E2%80%93Ullman_algorithm). A number of additions have invalidated Sethi-Ullman, but the allocator is Sethi-Ullman-like enough that, at JIT compile time, in the first (and only) pass, the allocator must know how many registers are used by each sub-tree. We have a generalization of Sethi-Ullman in mind that can do this correctly, but the current implementation hacks around this by always executing both branches of the conditional. In Bing, we do actually use conditionals, but they're always at the top level (e.g., the query plan looks at the version of a data structure, and then has different behavior depending on the version), so there's a hack to handle a top-level conditional that's sufficient for our internal use. Fixing conditionals to work "for real" is on our list of things to do, but it's relatively low priority unless we have a customer who needs conditionals.

Even in our side-effect-free world, some nodes have to be calculated before other nodes. That's because of something called StackVariable, which is like `let` in a functional laguage -- although it has no side effects, it must exist before it can be referenced.

It's important to take into account whether the code will run locally. For example, if you're running locally, you can take the address of a symbol in your c++ program and pass it in as a pointer literal. But there's guarantee that the symbol would be at the same address on another machine. It turns out that if you take the address of a standard library function, like `printf`, and use it on a Windows machine that's running similar software, it will often work, but that's a very bad idea.

Common subexpressions are only evaluated once. This is commonly used by us when traversing data structures. For example, if you access `foo.bar.baz` and `foo.bar.wat`, `foo.bar` is common, or if you're multiplying matrices. TODO: expand matrix example.

If you grep for `DESIGN NOTE` in the code, you can find explanations of other quirks in NativeJIT.

These notes should eventually be moved elsewhere and perhaps each be expanded into their own documents, but they're here for now as we figure out our documentation story.


## Commonly used methods

#### Immediates

These are simple types (e.g., `char` or `int`) or pointers to anything. This means that we can have, for example, pointers to structs but we can't have struct literals.

~~~
template <typename T> ImmediateNode<T>& Immediate(T value);
~~~

##### Examples

~~~
// Immediate.
Function<int64_t> exp1(allocator1, code1);

auto &imm1 = exp1.Immediate(1234ll);
auto fn1 = exp1.Compile(imm1);

assert(1234ll == fn1());
~~~


#### Unary Operators

~~~
template <typename TO, typename FROM> Node<TO>& Cast(Node<FROM>& value);
~~~

Pointer dereference; basically like `*`:

~~~
template <typename T> Node<T>& Deref(Node<T*>& pointer);
template <typename T> Node<T>& Deref(Node<T*>& pointer, int32_t index);
template <typename T> Node<T>& Deref(Node<T&>& reference);
~~~

Field de-reference; basically like `->`. If you have `a`, and apply the `b` FieldPointer, that's equivalent to `a->b`. There's no `.` because we don't have structs as value types.

If you have a reference to an object, you have to convert the reference to a pointer to apply this method. TODO: check the correctness of the previous statement. Note that this has no runtime cost.

~~~
template <typename OBJECT, typename FIELD, typename OBJECT1 = OBJECT>
Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT1::*field);
~~~

Because we have some operations that can only be done on pointers (or only done on references), we have `AsPointer` and `AsReference` to convert between pointer and reference. This is free in terms of actual runtime cost:

~~~
template <typename T> Node<T*>& AsPointer(Node<T&>& reference);
template <typename T> Node<T&>& AsReference(Node<T*>& pointer);
~~~

##### Examples

~~~
// Cast.
Function<int64_t> exp1(allocator1, code1);

auto &cast1 = exp1.Cast<float>(exp1.Immediate(10));
auto fn1 = exp1.Compile(cast1);

assert(float(10) == fn1());


// Access member via ->.
class Foo
{
public:
    uint32_t m_a;
    uint64_t m_b;
};

Function<uint64_t, Foo*> expression(allocator2, code2);

auto & a = expression.GetP1();
auto & b = expression.FieldPointer(a, &Foo::m_b);
auto & c = expression.Deref(b);
auto fn2 = expression.Compile(c);

Foo foo;
foo.m_b = 1234ull;
Foo* p1 = &foo;

assert(p1->m_b == fn2(p1));
~~~

#### Binary Operators

Binary artihmetic ops take either two nodes, or a node and an immediate. Note that although the types are templated as `L` and `R`, `L` and `R` should generally be the same for binary ops that take two nodes -- conversions must be made explicit. For `Rol`, `Shl`, and `Shr`, the immediate should be a `uint8_t`.

~~~
template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& And(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& Or(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);

template <typename L, typename R> Node<L>& Rol(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Shl(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Shr(Node<L>& left, R right);
~~~

Like `[]`, i.e., takes a pointer and adds an offset:

~~~
Node<T*>& Add(Node<T(*)[SIZE]>& array, Node<INDEX>& index);
template <typename T, typename INDEX> Node<T*>& Add(Node<T*>& array, Node<INDEX>& index);
~~~

##### Examples

~~~
// Array dereference with binary operation.

Function<uint64_t, uint64_t*> exp1(allocator1, code1);

auto & idx1 = exp1.Add(expression.GetP1(), expression.Immediate<uint64_t>(1ull));
auto & idx2 = exp1.Add(expression.GetP1(), expression.Immediate<uint64_t>(2ull));
auto & sum = exp1.Add(expression.Deref(a), expression.Deref(b));
auto fn1 = exp1.Compile(sum);

uint64_t array[10];
array[1] = 1;
array[2] = 128;

uint64_t * p1 = array;

assert(array[1] + array[2] == fn1(p1));
~~~



#### Compare & Conditional

Unlike other nodes, which return a generic T, compare nodes return a flag. The flag can be passed to a conditional, which takes a flag.

~~~
FlagExpressionNode<JCC>& Compare(Node<T>& left, Node<T>& right);

template <JccType JCC, typename T>
Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& trueValue, Node<T>& falseValue);

template <typename CONDT, typename T>
Node<T>& IfNotZero(Node<CONDT>& conditionValue, Node<T>& trueValue, Node<T>& falseValue);

template <typename T>
Node<T>& If(Node<bool>& conditionValue, Node<T>& thenValue, Node<T>& elseValue);
~~~

x86 conditional tests are available; a full list is available [here](http://unixwiz.net/techtips/x86-jumps.html).

##### Example

~~~
// JA (jump if above), i.e. unsigned ">"
Function<uint64_t, uint64_t, uint64_t> exp1(setup->GetAllocator(), setup->GetCode());

uint64_t trueValue = 5;
uint64_t falseValue = 6;

auto & a = expression.Compare<JccType::JA>(expression.GetP1(), expression.GetP2());
auto & b = expression.Conditional(a, expression.Immediate(trueValue), expression.Immediate(falseValue));
auto function = expression.Compile(b);

uint64_t p1 = 3;
uint64_t p2 = 4;

auto expected = (p1 > p2) ? trueValue : falseValue;
auto observed = function(p1, p2);

assert(expected == observed);

p1 = 5;
p2 = 4;

expected = (p1 > p2) ? trueValue : falseValue;
observed = function(p1, p2);

assert(expected == observed);
~~~

#### Call

Calls a C function.

~~~
template <typename R>
Node<R>& Call(Node<R (*)()>& function);

template <typename R, typename P1>
Node<R>& Call(Node<R (*)(P1)>& function, Node<P1>& param1);

template <typename R, typename P1, typename P2>
Node<R>& Call(Node<R (*)(P1, P2)>& function, Node<P1>& param1, Node<P2>& param2);

template <typename R, typename P1, typename P2, typename P3>
Node<R>& Call(Node<R (*)(P1, P2, P3)>& function,
         Node<P1>& param1,
         Node<P2>& param2,
         Node<P3>& param3);

template <typename R, typename P1, typename P2, typename P3, typename P4>
Node<R>& Call(Node<R (*)(P1, P2, P3, P4)>& function,
              Node<P1>& param1,
              Node<P2>& param2,
              Node<P3>& param3,
              Node<P4>& param4);
~~~


##### Examples

~~~
// Call SampleFunction.
int SampleFunction(int p1, int p2)
{
    return p1 + p2;
}

Function<int, int, int> exp1(allocator1, code1);

typedef int (*F)(int, int);

auto &imm1 = exp1.Immediate<F>(SampleFunction);
auto &call1 = exp1.Call(imm1, exp1.GetP1(), exp1.GetP2());
auto fn1 = exp1.Compile(call2);

assert(10+35 == fn1(10, 35));
~~~


## Rarely used methods

#### Unary methods


~~~
template <typename FROM> Node<FROM const>& AddConstCast(Node<FROM>& value);
template <typename FROM> Node<FROM>& RemoveConstCast(Node<FROM const>& value,
                                                     typename std::enable_if<!std::is_const<FROM>::value>::type* = nullptr);
template <typename FROM> Node<FROM&>& RemoveConstCast(Node<FROM const &>& value,
                                                      typename std::enable_if<std::is_const<FROM>::value>::type* = nullptr);

template <typename FROM> Node<FROM const *>& AddTargetConstCast(Node<FROM*>& value);
template <typename FROM> Node<FROM*>& RemoveTargetConstCast(Node<FROM const *>& value);
~~~

These sounds really weird, but they were useful for TODO.

#### Binary methods

~~~
Node<T>& Shld(Node<T>& shiftee, Node<T>& filler, uint8_t bitCount);
~~~

This is used for packed types (i.e., bitfields that get packed into 64-bits) to extract a bitfield.