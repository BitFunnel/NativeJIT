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
