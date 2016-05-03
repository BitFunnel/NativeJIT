#include <iostream>
#include <math.h>       // For value of e.
#include <string>
#include <sstream>

#include "NativeJIT/CodeGen/ExecutionBuffer.h"
#include "NativeJIT/CodeGen/FunctionBuffer.h"
#include "NativeJIT/Function.h"
#include "Temporary/Allocator.h"


using NativeJIT::Allocator;
using NativeJIT::ExecutionBuffer;
using NativeJIT::Function;
using NativeJIT::FunctionBuffer;


namespace Examples
{

    class Parser
    {
    public:
        Parser(std::string const & src,
               Allocator& allocator,
               FunctionBuffer& code);

        float Evaluate();

    private:
        NativeJIT::Node<float>& ParseSum();
        NativeJIT::Node<float>& ParseProduct();
        NativeJIT::Node<float>& ParseTerm();

        float ParseFloat();
        std::string ParseSymbol();

        bool IsFirstCharOfNumber(char c);

        void SkipWhite();
        void Consume(char c);

        char GetChar();
        char PeekChar();

        std::string const m_src;
        size_t m_currentPosition;

        Function<float> m_expression;
    };


    Parser::Parser(std::string const & src,
                   Allocator& allocator,
                   FunctionBuffer& code)
        : m_src(src),
          m_currentPosition(0),
          m_expression(allocator, code)
    {
    }


    float Parser::Evaluate()
    {
        auto& root = ParseSum();
        m_expression.Compile(root);
        auto function = m_expression.GetEntryPoint();
        return function();
    }


    NativeJIT::Node<float>& Parser::ParseSum()
    {
        auto& left = ParseProduct();

        SkipWhite();
        if (PeekChar() == '+')
        {
            GetChar();
            auto& right = ParseProduct();

            return m_expression.Add(left, right);
        }
        else if (PeekChar() == '-')
        {
            GetChar();
            auto& right = ParseProduct();

            return m_expression.Sub(left, right);
        }
        else
        {
            return left;
        }
    }


    NativeJIT::Node<float>& Parser::ParseProduct()
    {
        auto& left = ParseTerm();

        SkipWhite();
        if (PeekChar() == '*')
        {
            GetChar();
            auto& right = ParseSum();

            return m_expression.Mul(left, right);
        }
        else
        {
            return left;
        }
    }


    NativeJIT::Node<float>& Parser::ParseTerm()
    {
        SkipWhite();

        char next = PeekChar();
        if (next == '(')
        {
            GetChar();

            auto& result = ParseSum();

            SkipWhite();
            Consume(')');

            return result;
        }
        else if (IsFirstCharOfNumber(next))
        {
            float f = ParseFloat();
            return m_expression.Immediate(f);
        }
        else if (isalpha(next))
        {
            std::string symbol = ParseSymbol();
            if (symbol.compare("e") == 0)
            {
                // 'e' denotes Euler's number.
                const float e = static_cast<float>(exp(1));
                return m_expression.Immediate(e);
            }
            else if (symbol.compare("pi") == 0)
            {
                // 'pi' denotes the mathematical constant pi.
                const float pi = static_cast<float>(atan(1) * 4);
                return m_expression.Immediate(pi);
            }
            else if (symbol.compare("sqrt") == 0)
            {
                Consume('(');
                auto& parameter = ParseSum();
                Consume(')');
                auto & sqrtFunction = m_expression.Immediate(sqrtf);
                return m_expression.Call(sqrtFunction, parameter);
            }
            else
            {
                // TODO: REVIEW: Check lifetime of c_str() passed to exception constructor.
                std::stringstream message;
                message << "Unknown identifier \"" << symbol << "\".";
                throw std::exception(message.str().c_str());
            }
        }
        else
        {
            throw std::exception("Expected a number, symbol or parenthesized expression.");
        }
    }


    float Parser::ParseFloat()
    {
        std::string s;

        SkipWhite();

        // Optional leading '+' or '-'.
        if (PeekChar() == '+' || PeekChar() == '-')
        {
            s.push_back(GetChar());
        }

        // Optional mantissa left of '.'
        while (isdigit(PeekChar()))
        {
            s.push_back(GetChar());
        }

        // Optional portion of mantissa right of '.'.
        if (PeekChar() == '.')
        {
            s.push_back(GetChar());
            while (isdigit(PeekChar()))
            {
                s.push_back(GetChar());
            }
        }

        // Optional exponent.
        if (PeekChar() == 'e' || PeekChar() == 'E')
        {
            s.push_back(GetChar());

            // Optional '+' or '-' before exponent.
            if (PeekChar() == '+' || PeekChar() == '-')
            {
                s.push_back(GetChar());
            }

            if (!isdigit(PeekChar()))
            {
                throw std::exception("Expected exponent in floating point constant.");
            }

            while (isdigit(PeekChar()))
            {
                s.push_back(GetChar());
            }
        }

        return stof(s);
    }


    std::string Parser::ParseSymbol()
    {
        std::string symbol;

        SkipWhite();
        if (!isalpha(PeekChar()))
        {
            throw std::exception("Expected alpha character at beginning of symbol.");
        }
        while (isalnum(PeekChar()))
        {
            symbol.push_back(GetChar());
        }
        return symbol;
    }


    bool Parser::IsFirstCharOfNumber(char c)
    {
        return isdigit(c) || (c == '-') || (c == '+') || (c == '.');
    }


    void Parser::SkipWhite()
    {
        while (isspace(PeekChar()))
        {
            GetChar();
        }
    }


    void Parser::Consume(char c)
    {
        if (PeekChar() != c)
        {
            // TODO: REVIEW: Check lifetime of c_str() passed to exception constructor.
            std::stringstream message;
            message << "Expected '" << c << "'.";
            throw std::exception(message.str().c_str());
        }
        else
        {
            GetChar();
        }
    }


    char Parser::GetChar()
    {
        char result = PeekChar();
        if (result != '\0')
        {
            ++m_currentPosition;
        }
        return result;
    }


    char Parser::PeekChar()
    {
        if (m_currentPosition >= m_src.length())
        {
            return '\0';
        }
        else
        {
            return m_src[m_currentPosition];
        }
    }
}


bool Test()
{
    class TestCase
    {
    public:
        // TODO: REVIEW: Passing double here instead of float so that I don't
        // have to type the 'f' character at the end of every floating point
        // constant. Consider just changing entire example to use doubles.
        TestCase(char const * input, double output)
            : m_input(input),
            m_output(static_cast<float>(output))
        {
        }


        bool Run(std::ostream& output, Allocator& allocator, FunctionBuffer& code)
        {
            output << "\"" << m_input << "\" ==> ";

            Examples::Parser parser(m_input, allocator, code);
            float result = parser.Evaluate();

            output << result;

            if (result == m_output)
            {
                output << " OK";
            }
            else
            {
                output << " FAILED: expected " << m_output;
            }

            output << std::endl;

            return result == m_output;
        }

    private:
        char const * m_input;
        float m_output;
    };

    TestCase cases[] =
    {
        // Constants
        TestCase("1", 1.0),
        TestCase("1.234", 1.234),
        TestCase(".1", 0.1),
        TestCase("-2", -2.0),
        TestCase("-.1", -0.1),
        TestCase("1e9", 1e9),
        TestCase("2e-8", 2e-8),
        TestCase("3e+7", 3e+7),
        TestCase("456.789e+5", 456.789e+5),

        // Symbols
        TestCase("e", static_cast<float>(exp(1))),
        TestCase("pi", static_cast<float>(atan(1) * 4)),

        // Addition
        TestCase("1+2", 3.0),
        TestCase("3+e", 3.0 + static_cast<float>(exp(1))),

        // Subtraction
        TestCase("4-5", -1.0),

        // Multiplication
        TestCase("2*3", 6.0),

        // Parenthesized expressions
        TestCase("(3+4)", 7.0),
        TestCase("(3+4)*(2+3)", 35.0),

        // Combinations
        TestCase("1+-2", -1.0),     // Addition combined with unary negation.

        // White space
        TestCase("\t 1  + ( 2 * 10 )    ", 21.0),

        // sqrt
        TestCase("sqrt(4)", 2.0),
        TestCase("sqrt((3+4)*(2+3))", sqrtf(35)),
    };


    ExecutionBuffer codeAllocator(8192);
    Allocator allocator(8192);
    FunctionBuffer code(codeAllocator, 8192);

    bool success = true;
    for (size_t i = 0; i < sizeof(cases) / sizeof(TestCase); ++i)
    {
        allocator.Reset();
        codeAllocator.Reset();
        success &= cases[i].Run(std::cout, allocator, code);
    }

    return success;
}


int main()
{
    std::cout << "Running test cases ..." << std::endl;
    bool success = Test();
    if (success)
    {
        std::cout << "All tests succeeded." << std::endl;
    }
    else
    {
        std::cout << "One or more tests failed." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Type an expression and press return to evaluate." << std::endl;
    std::cout << "Enter an empty line to exit." << std::endl;

    ExecutionBuffer codeAllocator(8192);
    Allocator allocator(8192);
    FunctionBuffer code(codeAllocator, 8192);

    for (;;)
    {
        allocator.Reset();
        codeAllocator.Reset();

        std::string line;
        std::cout << ">> " << std::flush;
        std::getline(std::cin, line);

        // TODO: Should really see if line is completely blank.
        // Blank lines cause the parser to crash.
        if (line.length() == 0)
        {
            break;
        }

        Examples::Parser parser(line, allocator, code);
        float result = parser.Evaluate();
        std::cout << result << std::endl;
    }

    return 0;
}
