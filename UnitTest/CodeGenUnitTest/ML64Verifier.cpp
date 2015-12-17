#include "stdafx.h"

#include <iostream>             // TODO: Temporary - for debugging.

#ifdef USING_GTEST
#include "gtest/gtest.h"
#endif

#include "ML64Verifier.h"

#ifndef USING_GTEST
#include "SuiteCpp/UnitTest.h"
#define TEST(a, b) TestCase(b)
#endif


namespace NativeJIT
{
    // TODO: Add testLength parameter and verify that ml64Output has same length.
    // TODO: Print out helpful diagnostics on failure.
    ML64Verifier::ML64Verifier(char const * ml64Output, unsigned __int8 const * testOutput)
        : m_current(ml64Output),
            m_currentLine(0),
            m_currentLineStart(ml64Output),
            m_bytesVerified(0),
            m_testOutput(testOutput)
    {
        while (!AtEOF())
        {
            ProcessLine();
        }
    }


    void ML64Verifier::ProcessLine()
    {
        if (AdvanceToDataField())
        {
            while (!AtEOL())
            {
                if (isxdigit(PeekChar()))
                {
                    unsigned __int64 value;
                    unsigned size;

#ifdef USING_GTEST
                    ASSERT_TRUE(ReadHexNumber(value, size));
                    ASSERT_LE(size, 8u);
                    ASSERT_GT(size, 0u);
#else
                    TestAssert(ReadHexNumber(value, size));
                    TestAssert(size <= 8 && size > 0);
#endif

                    for (unsigned i = 0 ; i < size; ++i)
                    {
                        unsigned __int8 x = static_cast<unsigned __int8>(value & 0xffull);
                        std::cout.width(2);
                        std::cout.fill('0');
                        std::cout << std::hex << static_cast<unsigned>(x) << " ";
                        value = value >> 8;

                        if (x != m_testOutput[m_bytesVerified])
                        {
                            ReportError(x, m_testOutput[m_bytesVerified]);
                        }

#ifdef USING_GTEST
                        ASSERT_EQ(x, m_testOutput[m_bytesVerified]);
#else
                        TestAssert(x == m_testOutput[m_bytesVerified]);
#endif

                        ++m_bytesVerified;
                    }

                    SkipDelimiters();
                }
                else
                {
                    break;
                }
            }
        }

        AdvanceToNextLine();
    }


    void ML64Verifier::ReportError(unsigned expected, unsigned found)
    {
        std::cout << std::endl;
        std::cout << "ERROR: generated opcode does not match ml64 output." << std::endl;
        std::cout << "Line " << m_currentLine << std::endl;

        std::cout << '"';
        char const *line = m_currentLineStart;
        while (*line != '\0' && *line != '\n' && *line != '\r')
        {
            std::cout << *line++;
        }
        std::cout << '"' << std::endl;

        std::cout << "Expected ";
        std::cout.width(2);
        std::cout.fill('0');
        std::cout << std::hex << expected;

        std::cout << " - Found ";
        std::cout.width(2);
        std::cout.fill('0');
        std::cout << std::hex << found;

        std::cout << std::endl;
    }


    bool ML64Verifier::AdvanceToDataField()
    {
        if (PeekChar() == '\t')
        {
            GetChar();
            for (unsigned i = 0 ; i < 3; ++i)
            {
                if (PeekChar() != ' ')
                {
                    return false;
                }
                GetChar();
            }
        }
        else if (PeekChar() == ' ')
        {
            GetChar();
            for (unsigned i = 0 ; i < 8 ; ++i)
            {
                if (!isxdigit(PeekChar()) && PeekChar() != ' ')
                {
                    return false;
                }
                GetChar();
            }
            if (PeekChar() != ' ')
            {
                return false;
            }
            GetChar();
            if (PeekChar() != ' ')
            {
                return false;
            }
            GetChar();
        }
        return isxdigit(PeekChar()) != 0;
    }


    void ML64Verifier::SkipOffset()
    {
        if (!AtEOL() && PeekChar() == ' ')
        {
            GetChar();
            if (!AtEOL() && isxdigit(PeekChar()))
            {
                unsigned __int64 value;
                unsigned size;

#ifdef USING_GTEST
                ASSERT_TRUE(ReadHexNumber(value, size));
                ASSERT_EQ(4, size);
#else
                TestAssert(ReadHexNumber(value, size));
                TestAssert(size == 4);
#endif
            }
        }
    }


    void ML64Verifier::SkipDelimiters()
    {
        if (PeekChar() == '/' || PeekChar() == '|')
        {
            GetChar();
        }
        if (PeekChar() == ' ')
        {
            GetChar();
        }
    }


    void ML64Verifier::AdvanceToNextLine()
    {
        while (PeekChar() != '\0' && PeekChar() != '\n' && PeekChar() != '\r')
        {
            GetChar();
        }

        if (PeekChar() == '\n')
        {
            GetChar();
            if (PeekChar() == '\r')
            {
                GetChar();
            }
        }
        else if (PeekChar() == '\r')
        {
            GetChar();
            if (PeekChar() == '\n')
            {
                GetChar();
            }
        }
        m_currentLineStart = m_current;
        m_currentLine++;
    }


    bool ML64Verifier::ReadHexNumber(unsigned __int64& value, unsigned& size)
    {
        bool foundHexNumber = false;

        if (isxdigit(PeekChar()))
        {
            foundHexNumber = true;
            value = ReadHexByte();
            size = 1;

            for (unsigned i = 0 ; i < 7 ; ++i)
            {
                if (isxdigit(PeekChar()))
                {
                    value <<= 8;
                    value |= ReadHexByte();
                    ++size;
                }
                else
                {
                    break;
                }
            }
        }

        return foundHexNumber;
    }


    unsigned ML64Verifier::ReadHexByte()
    {
        return (ReadHexDigit() << 4) | ReadHexDigit();
    }


    unsigned ML64Verifier::ReadHexDigit()
    {
#ifdef USING_GTEST
        // Would like to use
        //   ASSERT_NE(0, isxdigit(PeekChar()));
        // but can't. See following article for explanation.
        //   http://stackoverflow.com/questions/12617806/assert-true-return-type-does-not-match-function-type-in-gtest
        // Instead use throw.
        if (isxdigit(PeekChar()) == 0)
        {
            throw 0;
        }
#else
        TestAssert(isxdigit(PeekChar()) != 0);
#endif

        char c = GetChar();
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            return c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F')
        {
            return c - 'A' + 10;
        }
        else
        {
#ifdef USING_GTEST
            // Force unit test to fail.
            throw 0;
#else
            TestFail();
            return 0;
#endif
        }
    }


    char ML64Verifier::GetChar()
    {
        char result = *m_current;
        if (result != '\0')
        {
            m_current++;
        }
        return result;
    }


    char ML64Verifier::PeekChar()
    {
        return *m_current;
    }


    bool ML64Verifier::AtEOL() const
    {
        return AtEOF() || *m_current == '\n' || *m_current == '\r';
    }


    bool ML64Verifier::AtEOF() const
    {
        return *m_current == '\0';
    }
}
