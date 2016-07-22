// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include <iomanip>
#include <iostream>

#include "ML64Verifier.h"
#include "TestSetup.h"


namespace NativeJIT
{
    // TODO: Add testLength parameter and verify that ml64Output has same length.
    // TODO: Print out helpful diagnostics on failure.
    ML64Verifier::ML64Verifier(char const * ml64Output,
                               uint8_t const * testOutput)
        : m_current(ml64Output),
          m_currentLine(0),
          m_currentLineStart(ml64Output),
          m_bytesVerified(0),
          m_testOutput(testOutput)
    {
        m_comparedBytes.fill('0');
        m_comparedBytes << std::uppercase << std::hex;

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
                    uint64_t value;
                    unsigned size;

                    LogThrowAssert(ReadHexNumber(value, size), "Failed to read hex number");
                    LogThrowAssert(size <= 8 && size > 0, "Invalid size %u", size);

                    for (unsigned i = 0 ; i < size; ++i)
                    {
                        uint8_t x = static_cast<uint8_t>(value & 0xffull);
                        m_comparedBytes << std::setw(2) << static_cast<unsigned>(x) << " ";
                        value = value >> 8;

                        if (x != m_testOutput[m_bytesVerified])
                        {
                            ReportError(x, m_testOutput[m_bytesVerified]);
                        }

                        LogThrowAssert(x == m_testOutput[m_bytesVerified], "Code mismatch");
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
        auto& out = std::cerr;
        IosMiniStateRestorer state(out);

        out.fill('0');

        out << std::uppercase << std::hex;

        out << "Compared bytes:" << std::endl;
        out << m_comparedBytes.str();
        out << std::endl;
        out << std::endl;
        out << "ERROR: generated opcode does not match ml64 output." << std::endl;
        out << "Line " << m_currentLine << std::endl;

        out << '"';
        char const *line = m_currentLineStart;
        while (*line != '\0' && *line != '\n' && *line != '\r')
        {
            out << *line++;
        }
        out << '"' << std::endl;

        out << "Expected ";
        out << std::setw(2) << expected;
        out << " - Found ";
        out << std::setw(2) << found;

        out << std::endl;
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
                uint64_t value;
                unsigned size;
                LogThrowAssert(ReadHexNumber(value, size), "Failed to read hex number");
                LogThrowAssert(size == 4, "Invalid size %u", size);
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


    bool ML64Verifier::ReadHexNumber(uint64_t& value, unsigned& size)
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
        if (isxdigit(PeekChar()) == 0)
        {
            char const * start = m_currentLineStart;
            AdvanceToNextLine();

            std::string line(start, m_currentLineStart);
            LogThrowAbort("Non-hex character in line: %s", line.c_str());
        }

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
            LogThrowAbort("Unexpected character, code: %u", c);
            throw std::runtime_error("Unreachable code");
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
