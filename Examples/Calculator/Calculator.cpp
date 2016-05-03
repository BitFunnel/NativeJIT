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
    Function<int64_t> expr(allocator, code);

    std::string tok;
    std::vector<int64_t> stack;
    std::vector<NativeJIT::Node<int64_t>*> jitStack;

    while (std::cin >> tok) {
        int64_t digits;
        if (std::istringstream(tok) >> digits) {
            stack.push_back(digits);
            jitStack.push_back(&expr.Immediate(digits));
        } else {
            if (stack.size() < 2) {
                std::cout << "Not enough operands. Please enter an operand.\n";
                continue;
            }

            if (tok == "+") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 + op2);


                auto right = jitStack.back(); jitStack.pop_back();
                auto left = jitStack.back(); jitStack.pop_back();
                jitStack.push_back(&(expr.Add(*left, *right)));
            } else if (tok == "*") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 * op2);

                auto right = jitStack.back(); jitStack.pop_back();
                auto left = jitStack.back(); jitStack.pop_back();
                jitStack.push_back(&(expr.Mul(*left, *right)));
            } else if (tok == "-") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 - op2);

                auto right = jitStack.back(); jitStack.pop_back();
                auto left = jitStack.back(); jitStack.pop_back();
                jitStack.push_back(&(expr.Sub(*left, *right)));
            } else if (tok == "|") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 | op2);

                auto right = jitStack.back(); jitStack.pop_back();
                auto left = jitStack.back(); jitStack.pop_back();
                jitStack.push_back(&(expr.Or(*left, *right)));
            } else if (tok == "&") {
                int64_t op2 = stack.back(); stack.pop_back();
                int64_t op1 = stack.back(); stack.pop_back();
                stack.push_back(op1 & op2);

                auto right = jitStack.back(); jitStack.pop_back();
                auto left = jitStack.back(); jitStack.pop_back();
                jitStack.push_back(&(expr.And(*left, *right)));
            } else {
                std::cout << "Invalid input: " << tok << std::endl;
            }
        }
    }

    std::cout << "Compiling" << std::endl;
    auto fn = expr.Compile(*jitStack.back());
    auto res = fn();
    std::cout << "JIT result:normal result -- " << res << ":" << stack.back() << std::endl;
    assert(res == stack.back());
    return stack.back();
}

int main() {
    std::cout << calc() << std::endl;
}
