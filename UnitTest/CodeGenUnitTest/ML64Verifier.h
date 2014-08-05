#pragma once


namespace NativeJIT
{
    // TODO: Print out line that doesn't match.
    class ML64Verifier
    {
    public:
        ML64Verifier(char const * ml64Output, unsigned __int8 const * testOutput);

    private:
        void ProcessLine();
        bool AdvanceToDataField();
        void SkipOffset();
        void SkipDelimiters();
        void AdvanceToNextLine();
        bool ReadHexNumber(unsigned __int64& value, unsigned& size);
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
        unsigned __int8 const * m_testOutput;
    };
}
