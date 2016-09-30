#include <iostream>
#include <math.h>       // For value of e.
#include <stdexcept>
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
        //
        // Constructs a parser for the expression text in src.
        // Allocator and FunctionBuffer are constructor parameters
        // for NativeJIT::Function.
        //
        Parser(std::string const & src,
               Allocator& allocator,
               FunctionBuffer& code);

        //
        // Compiles the expression, then invokes the resulting
        // function.
        //
        float Evaluate();


        //
        // ParseError records the character position and cause of an error
        // during parsing.
        //
        class ParseError : public std::runtime_error
        {
        public:
            ParseError(char const * message, size_t position);

            friend std::ostream& operator<< (std::ostream &out, const ParseError &e);

        private:
            // Character position where error occurred.
            size_t m_position;
        };

    private:
        // Parses an expression of the form
        // EXPRESSION:
        //   SUM
        NativeJIT::Node<float>& Parse();

        // Parses expressions of form
        // SUM:
        //   PRODUCT ('+' PRODUCT)*
        //   PRODUCT ('-' PRODUCT)*
        NativeJIT::Node<float>& ParseSum();

        // Parses expressions of the form
        // PRODUCT:
        //   TERM ('*' TERM)*
        NativeJIT::Node<float>& ParseProduct();

        // Parses expressions of the form
        // TERM:
        //   (SUM)
        //   FLOAT
        //   SYMBOL
        NativeJIT::Node<float>& ParseTerm();

        // Parses expressions of the form
        // FLOAT:
        //   [ '+' | '-' ] (DIGIT)* [ '.' DIGIT*] [ ('e' | 'E') [ '+' | '-' ] DIGIT* ]
        float ParseFloat();

        // Parses expressions of the form
        // SYMBOL: ALPHA (ALPHA | DIGIT)*
        std::string ParseSymbol();

        // Returns true if current position is the first character of a floating
        // point number.
        bool IsFirstCharOfFloat(char c);

        // Advances the current position past whitespace (space, tab, carriage
        // return, newline).
        void SkipWhite();

        // Attempts to advance past next character.
        // Throws a ParseError if the next character is not equal to
        // the expected character.
        void Consume(char expected);

        // Advances past the next character.
        // Returns the character or '\0' if at the end of the stream.
        char GetChar();

        // Returns the next character without advancing.
        // Returns '\0' if at the end of the stream.
        char PeekChar();

        // Source string to be parsed.
        std::string const m_src;

        // Current position of parser in the m_src.
        size_t m_currentPosition;

        // NativeJIT Function used to build and compile parsed expression.
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
        auto& root = Parse();
        m_expression.Compile(root);
        auto function = m_expression.GetEntryPoint();
        return function();
    }


    NativeJIT::Node<float>& Parser::Parse()
    {
        auto& expression = ParseSum();

        SkipWhite();
        if (PeekChar() != '\0')
        {
            throw ParseError("Syntax error.", m_currentPosition);
        }

        return expression;
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
        else if (IsFirstCharOfFloat(next))
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
                throw ParseError(message.str().c_str(), m_currentPosition);
            }
        }
        else
        {
            throw ParseError("Expected a number, symbol or parenthesized expression.",
                             m_currentPosition);
        }
    }


    float Parser::ParseFloat()
    {
        // s will hold a string of floating point number characters that will
        // eventually be passed to stof().
        std::string s;

        SkipWhite();

        //
        // Gather in s the longest possible sequence of characters that will
        // parse as a floating point number.
        //

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
                throw ParseError("Expected exponent in floating point constant.",
                                 m_currentPosition);
            }

            while (isdigit(PeekChar()))
            {
                s.push_back(GetChar());
            }
        }

        // Parse s into a floating point value.
        // TODO: could check for really obvious errors before the try.
        try
        {
            return stof(s);
        }
        catch (...)
        {
            throw ParseError("Invalid float.",
                             m_currentPosition);
        }
    }


    std::string Parser::ParseSymbol()
    {
        std::string symbol;

        SkipWhite();
        if (!isalpha(PeekChar()))
        {
            throw ParseError("Expected alpha character at beginning of symbol.",
                             m_currentPosition);
        }
        while (isalnum(PeekChar()))
        {
            symbol.push_back(GetChar());
        }
        return symbol;
    }


    bool Parser::IsFirstCharOfFloat(char c)
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
            throw ParseError(message.str().c_str(), m_currentPosition);
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


    Parser::ParseError::ParseError(char const * message, size_t position)
        : std::runtime_error(message),
          m_position(position)
    {
    }


    std::ostream& operator<< (std::ostream &out, const Parser::ParseError &e)
    {
        out << std::string(e.m_position, ' ') << '^' << std::endl;
        out << "Parser error (position = " << e.m_position << "): ";
        out << e.what();
        out << std::endl;
        return out;
    }
}



/******************************************************************************
 *
 * Test() runs a number of test cases for the parser.
 * It prints a summary of each case's input and output and either "OK"
 * or "FAILED" depending on whether the test succeeded or failed.
 *
 * Returns true if all tests pass. Otherwise returns false.
 *
 ******************************************************************************/
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
            bool succeeded = false;

            output << "\"" << m_input << "\" ==> ";

            try {
                Examples::Parser parser(m_input, allocator, code);
                float result = parser.Evaluate();

                output << result;

                if (result == m_output)
                {
                    succeeded = true;
                    output << " OK";
                }
                else
                {
                    output << " FAILED: expected " << m_output;
                }
            } catch (...) {
                output << "FAILED: exception.";
            }

            output << std::endl;

            return succeeded;
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
    std::string prompt(">> ");

    for (;;)
    {
        allocator.Reset();
        codeAllocator.Reset();

        std::string line;
        std::cout << prompt << std::flush;
        std::getline(std::cin, line);

        // TODO: Should really see if line is completely blank.
        // Blank lines cause the parser to crash.
        if (line.length() == 0)
        {
            break;
        }

        try
        {
            Examples::Parser parser(line, allocator, code);
            float result = parser.Evaluate();
            std::cout << result << std::endl;
        }
        catch (Examples::Parser::ParseError& e)
        {
            std::cout << std::string(prompt.length(), ' ');
            std::cout << e;
        }
    }

    return 0;
}
