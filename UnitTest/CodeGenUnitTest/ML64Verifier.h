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


#pragma once

#include <cstdint>
#include <sstream>


namespace NativeJIT
{
    class ML64Verifier
    {
    public:
        ML64Verifier(char const * ml64Output, uint8_t const * testOutput);

    private:
        void ProcessLine();
        void ReportError(unsigned expected, unsigned found);
        bool AdvanceToDataField();
        void SkipOffset();
        void SkipDelimiters();
        void AdvanceToNextLine();
        bool ReadHexNumber(uint64_t& value, unsigned& size);
        unsigned ReadHexByte();
        unsigned ReadHexDigit();

        char GetChar();
        char PeekChar();

        bool AtEOL() const;
        bool AtEOF() const;

        char const * m_current;
        unsigned m_currentLine;
        char const * m_currentLineStart;

        unsigned m_bytesVerified;
        uint8_t const * m_testOutput;
        std::stringstream m_comparedBytes;
    };
}
