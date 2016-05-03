#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using NativeJIT::Allocator;
using NativeJIT::ExecutionBuffer;
using NativeJIT::Function;
using NativeJIT::FunctionBuffer;

// TODO: refactor so that we push JIT nodes the same way we do for the
// non-JIT'ed version.
int64_t calc() {
    ExecutionBuffer codeAllocator(8192);
    Allocator allocator(8192);
    FunctionBuffer code(codeAllocator, 8192);
    Function<int64_t, int64_t, int64_t> expr(allocator, code);

    int numArgs = 2;
    int curArg = 0;

    std::string tok;
    std::vector<int64_t> stack;
    NativeJIT::Node<int64_t> *temp = NULL;

    std::vector<int64_t> args;
    while (std::cin >> tok) {
        int64_t digits;
        if (std::istringstream(tok) >> digits) {
            if (curArg < numArgs) {
                stack.push_back(digits);
                args.push_back(digits);
                ++curArg;
                std::cout << "Add token to stack" << std::endl;
            } else {
                std::cout << "Out of arguments. Please enter an operator.\n";
            }
        } else {
            if (stack.size() < 2) {
                std::cout << "Not enough operands. Please enter an operand.\n";
                continue;
            }

            if (tok == "+") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 + op2);
                --curArg;
                std::cout << "Add node" << std::endl;
                temp = &expr.Add(expr.GetP1(), expr.GetP2());
            } else if (tok == "*") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 * op2);
                --curArg;
                std::cout << "Mul node" << std::endl;
                temp = &expr.Mul(expr.GetP1(), expr.GetP2());
            } else {
                std::cout << "Invalid input: " << tok << std::endl;
            }
        }
    }

    std::cout << "Compiling" << std::endl;
    auto fn = expr.Compile(*temp);
    int64_t res = fn(args[0], args[1]);
    std::cout << "JIT result:normal result -- " << res << ":" << stack.back() << std::endl;
    assert(res == stack.back());
    return stack.back();
}

int main() {
    std::cout << calc() << std::endl;
}
