## Parser Example

This example demonstrates a compiler for infix expressions.
For simplicity, the sample only supports floats
(as of the first README.md commit, 
the compiler itself is just 300 lines of C++ code!)

Expressions can be composed of 

* Floating point constants like 1, -2, 3.4, 5.67e-9.
* The symbols e and pi which map to the well known constants.
* Operators +, -, *
* Parentheses ().
* The sqrt() function.

The sample executable first runs a set of unit tests,
which also serve to give an example of the type of expressions
that can be compiled.

After running the tests, it falls into a Read-Eval-Print loop
which prints out a >> prompt and then waits for input. At this
point you can type an expression and hit enter to evaluate it.

Entering a blank line will exit the program. Here's a sample run:

~~~
Running test cases ...
"1" ==> 1 OK
"1.234" ==> 1.234 OK
".1" ==> 0.1 OK
"-2" ==> -2 OK
"-.1" ==> -0.1 OK
"1e9" ==> 1e+09 OK
"2e-8" ==> 2e-08 OK
"3e+7" ==> 3e+07 OK
"456.789e+5" ==> 4.56789e+07 OK
"e" ==> 2.71828 OK
"pi" ==> 3.14159 OK
"1+2" ==> 3 OK
"3+e" ==> 5.71828 OK
"4-5" ==> -1 OK
"2*3" ==> 6 OK
"(3+4)" ==> 7 OK
"(3+4)*(2+3)" ==> 35 OK
"1+-2" ==> -1 OK
"        1  + ( 2 * 10 )    " ==> 21 OK
"sqrt(4)" ==> 2 OK
"sqrt((3+4)*(2+3))" ==> 5.91608 OK
All tests succeeded.

Type an expression and press return to evaluate.
Enter an empty line to exit.
>> 12345 * 65432
8.07758e+08
>> sqrt(100)
10
>>
~~~
