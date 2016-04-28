## Description of public headers

### Create the function

The are three parameters: argument types, allocator, and code buffer. 

1. Template type parameters define return and argument types for your function. Can have 0 to 4 arguments.


For example, if you're providing a function that computes the area of a circle, it might be something like:

~~~
float area(float radius);
~~~

This would correspond to

~~~
<float, float>
~~~

The first parameter is the return value and the remaining parmeters are the return types.

2. The allocator provides the memory where expression nodes will reside. There are a lot of ways you can do this, but a reasonable default is the allocator in `Temporary` directory.

~~~
ExecutionBuffer codeAllocator(8192);
Allocator allocator(8192);
~~~~

Note that these sizes don't necessarily have to be the same.

3. The code buffer provides executable memory where the compile code will reside.

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

### ???

No side effects.

It's important to take into account whether the code will run locally. For example, if you're running locally, you can take the address of a symbol in your c++ program and pass it in as a pointer literal. But there's guarantee that the symbol would be at the same address on another machine.

Dependent node. Some nodes have to be calculated before other nodes. This is true even in our side-effect-free world. That's because we have StackVariable, which is like `let` in a functional laguage -- although it has no side effects, it must exist before it can be referenced.

######## Common Subexpressions

Common subexpressions are only evaluated once. This is commonly used by us when traversing data structures. For example, if you access `foo.bar.baz` and `foo.bar.wat`, `foo.bar` is common, or if you're multiplying matrices. TODO: expand matrix example.

### end ???


#### Immediates

These are simple types (e.g., `char` or `int`) or pointers to anything.

template <typename T> ImmediateNode<T>& Immediate(T value);

#### Parameters



#### Unary Operators

TODO: refactor this into common operators and weird suff that most people will never touch.

Like casts, except that C++ doesn't have a "normal" way of doing this, although it's technically possible

~~~
template <typename T> Node<T*>& AsPointer(Node<T&>& reference);
template <typename T> Node<T&>& AsReference(Node<T*>& pointer);
~~~

~~~
template <typename TO, typename FROM> Node<TO>& Cast(Node<FROM>& value);
~~~


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

~~~
template <typename T> Node<T>& Deref(Node<T*>& pointer);
template <typename T> Node<T>& Deref(Node<T*>& pointer, int32_t index);
template <typename T> Node<T>& Deref(Node<T&>& reference);
~~~

Basically like `*`. 

~~~
template <typename OBJECT, typename FIELD, typename OBJECT1 = OBJECT>
Node<FIELD*>& FieldPointer(Node<OBJECT*>& object, FIELD OBJECT1::*field);
~~~

FieldPointer is like `->` on a struct. If you have `a`, and apply the `b` FieldPointer, that's equivalent to `a->b`. There's no `.` because we don't have structs as value types.

If you have a reference to an object, you have to convert the reference to a pointer to apply this method. TODO: check the correctness of the previous statement. Note that this has no runtime cost.

#### Binary Operators

~~~
template <typename L, typename R> Node<L>& Add(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& And(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& Mul(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& MulImmediate(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Or(Node<L>& left, Node<R>& right);
template <typename L, typename R> Node<L>& Rol(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Shl(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Shr(Node<L>& left, R right);
template <typename L, typename R> Node<L>& Sub(Node<L>& left, Node<R>& right);
~~~

TODO: will types auto-covert? e.g., can L and R be different types?

~~~
Node<T*>& Add(Node<T(*)[SIZE]>& array, Node<INDEX>& index);
template <typename T, typename INDEX> Node<T*>& Add(Node<T*>& array, Node<INDEX>& index);
~~~

Like `[]`, i.e., takes a pointer and adds an offset.

~~~
Node<T>& Shld(Node<T>& shiftee, Node<T>& filler, uint8_t bitCount);
~~~

This is used for packed types (i.e., bitfields that get packed into 64-bits) to extract a bitfield.

#### Compare

Unlike other nodes, which return a generic T, compare nodes return a flag. The flag can be passed to a conditional, which takes a flag.

~~~
FlagExpressionNode<JCC>& Compare(Node<T>& left, Node<T>& right);
~~~

#### Conditional

These are a hack and we haven't implemented the fix yet. The hack is that both sides of the conditional are taken, and then the correct value is returned. This is safe because everything is side-effect free. We have a fix in mind, but it requires changing the register allocation algorithm, which is non-trivial.

~~~
template <JccType JCC, typename T>
Node<T>& Conditional(FlagExpressionNode<JCC>& condition, Node<T>& trueValue, Node<T>& falseValue);

template <typename CONDT, typename T>
Node<T>& IfNotZero(Node<CONDT>& conditionValue, Node<T>& trueValue, Node<T>& falseValue);

template <typename T>
Node<T>& If(Node<bool>& conditionValue, Node<T>& thenValue, Node<T>& elseValue);
~~~

There's a workaround for this that works for how this is used in Bing, but it only works at the top level of the function. This was done because we have code that checks the version number, and then does something based on that, and that's the only conditional Bing uses. This obviously isn't general and we will eventually fix this.

#### Call

Calls a C function. The C function shouldn't cause side effects. The function will be called at least once.

At least once because of the conditional hack, above. Additionally, because we assume that things are side-effect free, optimizations may cause things to be re-ordered.

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