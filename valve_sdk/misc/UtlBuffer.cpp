//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// $Header: $
// $NoKeywords: $
//
// Serialization buffer
//===========================================================================//

#pragma warning (disable : 4514)

#include "UtlBuffer.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include "characterset.hpp"

const char* V_strnchr(const char* pStr, char c, int n)
{
    char const* pLetter = pStr;
    char const* pLast = pStr + n;

    // Check the entire string
    while((pLetter < pLast) && (*pLetter != 0)) {
        if(*pLetter == c)
            return pLetter;
        ++pLetter;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
// Finds a string in another string with a case insensitive test w/ length validation
//-----------------------------------------------------------------------------
char const* V_strnistr(char const* pStr, char const* pSearch, int n)
{
    if(!pStr || !pSearch)
        return 0;

    char const* pLetter = pStr;

    // Check the entire string
    while(*pLetter != 0) {
        if(n <= 0)
            return 0;

        // Skip over non-matches
        if(tolower(*pLetter) == tolower(*pSearch)) {
            int n1 = n - 1;

            // Check for match
            char const* pMatch = pLetter + 1;
            char const* pTest = pSearch + 1;
            while(*pTest != 0) {
                if(n1 <= 0)
                    return 0;

                // We've run off the end; don't bother.
                if(*pMatch == 0)
                    return 0;

                if(tolower(*pMatch) != tolower(*pTest))
                    break;

                ++pMatch;
                ++pTest;
                --n1;
            }

            // Found a match!
            if(*pTest == 0)
                return pLetter;
        }

        ++pLetter;
        --n;
    }

    return 0;
}
//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion
{
public:
    CUtlCStringConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray);

    // Finds a conversion for the passed-in string, returns length
    virtual char FindConversion(const char *pString, int *pLength);

private:
    char m_pConversion[255];
};


//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion
{
public:
    CUtlNoEscConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray) :
        CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray)
    {
    }

    // Finds a conversion for the passed-in string, returns length
    virtual char FindConversion(const char *pString, int *pLength) { *pLength = 0; return 0; }
};


//-----------------------------------------------------------------------------
// List of character conversions
//-----------------------------------------------------------------------------
BEGIN_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\')
{
    '\n', "n"
},
{ '\t', "t" },
{ '\v', "v" },
{ '\b', "b" },
{ '\r', "r" },
{ '\f', "f" },
{ '\a', "a" },
{ '\\', "\\" },
{ '\?', "\?" },
{ '\'', "\'" },
{ '\"', "\"" },
END_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\');

    CUtlCharConversion *GetCStringCharConversion()
    {
        return &s_StringCharConversion;
    }

    BEGIN_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F)
    {
        0x7F, ""
    },
        END_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F);

        CUtlCharConversion *GetNoEscCharConversion()
        {
            return &s_NoEscConversion;
        }


        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        CUtlCStringConversion::CUtlCStringConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray) :
            CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray)
        {
            memset(m_pConversion, 0x0, sizeof(m_pConversion));
            for(int i = 0; i < nCount; ++i) {
                m_pConversion[pArray[i].m_pReplacementString[0]] = pArray[i].m_nActualChar;
            }
        }

        // Finds a conversion for the passed-in string, returns length
        char CUtlCStringConversion::FindConversion(const char *pString, int *pLength)
        {
            char c = m_pConversion[pString[0]];
            *pLength = (c != '\0') ? 1 : 0;
            return c;
        }



        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        CUtlCharConversion::CUtlCharConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray)
        {
            m_nEscapeChar = nEscapeChar;
            m_pDelimiter = pDelimiter;
            m_nCount = nCount;
            m_nDelimiterLength = strlen(pDelimiter);
            m_nMaxConversionLength = 0;

            memset(m_pReplacements, 0, sizeof(m_pReplacements));

            for(int i = 0; i < nCount; ++i) {
                m_pList[i] = pArray[i].m_nActualChar;
                ConversionInfo_t &info = m_pReplacements[m_pList[i]];
                assert(info.m_pReplacementString == 0);
                info.m_pReplacementString = pArray[i].m_pReplacementString;
                info.m_nLength = strlen(info.m_pReplacementString);
                if(info.m_nLength > m_nMaxConversionLength) {
                    m_nMaxConversionLength = info.m_nLength;
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Escape character + delimiter
        //-----------------------------------------------------------------------------
        char CUtlCharConversion::GetEscapeChar() const
        {
            return m_nEscapeChar;
        }

        const char *CUtlCharConversion::GetDelimiter() const
        {
            return m_pDelimiter;
        }

        int CUtlCharConversion::GetDelimiterLength() const
        {
            return m_nDelimiterLength;
        }


        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        const char *CUtlCharConversion::GetConversionString(char c) const
        {
            return m_pReplacements[c].m_pReplacementString;
        }

        int CUtlCharConversion::GetConversionLength(char c) const
        {
            return m_pReplacements[c].m_nLength;
        }

        int CUtlCharConversion::MaxConversionLength() const
        {
            return m_nMaxConversionLength;
        }


        //-----------------------------------------------------------------------------
        // Finds a conversion for the passed-in string, returns length
        //-----------------------------------------------------------------------------
        char CUtlCharConversion::FindConversion(const char *pString, int *pLength)
        {
            for(int i = 0; i < m_nCount; ++i) {
                if(!strcmp(pString, m_pReplacements[m_pList[i]].m_pReplacementString)) {
                    *pLength = m_pReplacements[m_pList[i]].m_nLength;
                    return m_pList[i];
                }
            }

            *pLength = 0;
            return '\0';
        }


        //-----------------------------------------------------------------------------
        // constructors
        //-----------------------------------------------------------------------------
        CUtlBuffer::CUtlBuffer(int growSize, int initSize, int nFlags) :
            m_Memory(growSize, initSize), m_Error(0)
        {
            m_Get = 0;
            m_Put = 0;
            m_nTab = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            if((initSize != 0) && !IsReadOnly()) {
                m_nMaxPut = -1;
                AddNullTermination();
            } else {
                m_nMaxPut = 0;
            }
            SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
        }

        CUtlBuffer::CUtlBuffer(const void *pBuffer, int nSize, int nFlags) :
            m_Memory((unsigned char*)pBuffer, nSize), m_Error(0)
        {
            assert(nSize != 0);

            m_Get = 0;
            m_Put = 0;
            m_nTab = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            if(IsReadOnly()) {
                m_nMaxPut = nSize;
            } else {
                m_nMaxPut = -1;
                AddNullTermination();
            }
            SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
        }


        //-----------------------------------------------------------------------------
        // Modifies the buffer to be binary or text; Blows away the buffer and the CONTAINS_CRLF value. 
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetBufferType(bool bIsText, bool bContainsCRLF)
        {
#ifdef _DEBUG
            // If the buffer is empty, there is no opportunity for this stuff to fail
            if(TellMaxPut() != 0) {
                if(IsText()) {
                    if(bIsText) {
                        assert(ContainsCRLF() == bContainsCRLF);
                    } else {
                        assert(ContainsCRLF());
                    }
                } else {
                    if(bIsText) {
                        assert(bContainsCRLF);
                    }
                }
            }
#endif

            if(bIsText) {
                m_Flags |= TEXT_BUFFER;
            } else {
                m_Flags &= ~TEXT_BUFFER;
            }
            if(bContainsCRLF) {
                m_Flags |= CONTAINS_CRLF;
            } else {
                m_Flags &= ~CONTAINS_CRLF;
            }
        }


        //-----------------------------------------------------------------------------
        // Attaches the buffer to external memory....
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetExternalBuffer(void* pMemory, int nSize, int nInitialPut, int nFlags)
        {
            m_Memory.SetExternalBuffer((unsigned char*)pMemory, nSize);

            // Reset all indices; we just changed memory
            m_Get = 0;
            m_Put = nInitialPut;
            m_nTab = 0;
            m_Error = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            m_nMaxPut = -1;
            AddNullTermination();
        }

        //-----------------------------------------------------------------------------
        // Assumes an external buffer but manages its deletion
        //-----------------------------------------------------------------------------
        void CUtlBuffer::AssumeMemory(void *pMemory, int nSize, int nInitialPut, int nFlags)
        {
            m_Memory.AssumeMemory((unsigned char*)pMemory, nSize);

            // Reset all indices; we just changed memory
            m_Get = 0;
            m_Put = nInitialPut;
            m_nTab = 0;
            m_Error = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            m_nMaxPut = -1;
            AddNullTermination();
        }

        //-----------------------------------------------------------------------------
        // Makes sure we've got at least this much memory
        //-----------------------------------------------------------------------------
        void CUtlBuffer::EnsureCapacity(int num)
        {
            // Add one extra for the null termination
            num += 1;
            if(m_Memory.IsExternallyAllocated()) {
                if(IsGrowable() && (m_Memory.NumAllocated() < num)) {
                    m_Memory.ConvertToGrowableMemory(0);
                } else {
                    num -= 1;
                }
            }

            m_Memory.EnsureCapacity(num);
        }


        //-----------------------------------------------------------------------------
        // Base Get method from which all others derive
        //-----------------------------------------------------------------------------
        void CUtlBuffer::Get(void* pMem, int size)
        {
            if(CheckGet(size)) {
                memcpy(pMem, &m_Memory[m_Get - m_nOffset], size);
                m_Get += size;
            }
        }


        //-----------------------------------------------------------------------------
        // This will Get at least 1 uint8_t and up to nSize bytes. 
        // It will return the number of bytes actually read.
        //-----------------------------------------------------------------------------
        int CUtlBuffer::GetUpTo(void *pMem, int nSize)
        {
            if(CheckArbitraryPeekGet(0, nSize)) {
                memcpy(pMem, &m_Memory[m_Get - m_nOffset], nSize);
                m_Get += nSize;
                return nSize;
            }
            return 0;
        }


        //-----------------------------------------------------------------------------
        // Eats whitespace
        //-----------------------------------------------------------------------------
        void CUtlBuffer::EatWhiteSpace()
        {
            if(IsText() && IsValid()) {
                while(CheckGet(sizeof(char))) {
                    if(!isspace(*(const unsigned char*)PeekGet()))
                        break;
                    m_Get += sizeof(char);
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Eats C++ style comments
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::EatCPPComment()
        {
            if(IsText() && IsValid()) {
                // If we don't have a a c++ style comment next, we're done
                const char *pPeek = (const char *)PeekGet(2 * sizeof(char), 0);
                if(!pPeek || (pPeek[0] != '/') || (pPeek[1] != '/'))
                    return false;

                // Deal with c++ style comments
                m_Get += 2;

                // read complete line
                for(char c = GetChar(); IsValid(); c = GetChar()) {
                    if(c == '\n')
                        break;
                }
                return true;
            }
            return false;
        }


        //-----------------------------------------------------------------------------
        // Peeks how much whitespace to eat
        //-----------------------------------------------------------------------------
        int CUtlBuffer::PeekWhiteSpace(int nOffset)
        {
            if(!IsText() || !IsValid())
                return 0;

            while(CheckPeekGet(nOffset, sizeof(char))) {
                if(!isspace(*(unsigned char*)PeekGet(nOffset)))
                    break;
                nOffset += sizeof(char);
            }

            return nOffset;
        }


        //-----------------------------------------------------------------------------
        // Peek size of sting to come, check memory bound
        //-----------------------------------------------------------------------------
        int	CUtlBuffer::PeekStringLength()
        {
            if(!IsValid())
                return 0;

            // Eat preceeding whitespace
            int nOffset = 0;
            if(IsText()) {
                nOffset = PeekWhiteSpace(nOffset);
            }

            int nStartingOffset = nOffset;

            do {
                int nPeekAmount = 128;

                // NOTE: Add 1 for the terminating zero!
                if(!CheckArbitraryPeekGet(nOffset, nPeekAmount)) {
                    if(nOffset == nStartingOffset)
                        return 0;
                    return nOffset - nStartingOffset + 1;
                }

                const char *pTest = (const char *)PeekGet(nOffset);

                if(!IsText()) {
                    for(int i = 0; i < nPeekAmount; ++i) {
                        // The +1 here is so we eat the terminating 0
                        if(pTest[i] == 0)
                            return (i + nOffset - nStartingOffset + 1);
                    }
                } else {
                    for(int i = 0; i < nPeekAmount; ++i) {
                        // The +1 here is so we eat the terminating 0
                        if(isspace((unsigned char)pTest[i]) || (pTest[i] == 0))
                            return (i + nOffset - nStartingOffset + 1);
                    }
                }

                nOffset += nPeekAmount;

            } while(true);
        }


        //-----------------------------------------------------------------------------
        // Peek size of line to come, check memory bound
        //-----------------------------------------------------------------------------
        int	CUtlBuffer::PeekLineLength()
        {
            if(!IsValid())
                return 0;

            int nOffset = 0;
            int nStartingOffset = nOffset;

            do {
                int nPeekAmount = 128;

                // NOTE: Add 1 for the terminating zero!
                if(!CheckArbitraryPeekGet(nOffset, nPeekAmount)) {
                    if(nOffset == nStartingOffset)
                        return 0;
                    return nOffset - nStartingOffset + 1;
                }

                const char *pTest = (const char *)PeekGet(nOffset);

                for(int i = 0; i < nPeekAmount; ++i) {
                    // The +2 here is so we eat the terminating '\n' and 0
                    if(pTest[i] == '\n' || pTest[i] == '\r')
                        return (i + nOffset - nStartingOffset + 2);
                    // The +1 here is so we eat the terminating 0
                    if(pTest[i] == 0)
                        return (i + nOffset - nStartingOffset + 1);
                }

                nOffset += nPeekAmount;

            } while(true);
        }


        //-----------------------------------------------------------------------------
        // Does the next bytes of the buffer match a pattern?
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::PeekStringMatch(int nOffset, const char *pString, int nLen)
        {
            if(!CheckPeekGet(nOffset, nLen))
                return false;
            return !strncmp((const char*)PeekGet(nOffset), pString, nLen);
        }


        //-----------------------------------------------------------------------------
        // This version of PeekStringLength converts \" to \\ and " to \, etc.
        // It also reads a " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        int CUtlBuffer::PeekDelimitedStringLength(CUtlCharConversion *pConv, bool bActualSize)
        {
            if(!IsText() || !pConv)
                return PeekStringLength();

            // Eat preceeding whitespace
            int nOffset = 0;
            if(IsText()) {
                nOffset = PeekWhiteSpace(nOffset);
            }

            if(!PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                return 0;

            // Try to read ending ", but don't accept \"
            int nActualStart = nOffset;
            nOffset += pConv->GetDelimiterLength();
            int nLen = 1;	// Starts at 1 for the '\0' termination

            do {
                if(PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                    break;

                if(!CheckPeekGet(nOffset, 1))
                    break;

                char c = *(const char*)PeekGet(nOffset);
                ++nLen;
                ++nOffset;
                if(c == pConv->GetEscapeChar()) {
                    int nLength = pConv->MaxConversionLength();
                    if(!CheckArbitraryPeekGet(nOffset, nLength))
                        break;

                    pConv->FindConversion((const char*)PeekGet(nOffset), &nLength);
                    nOffset += nLength;
                }
            } while(true);

            return bActualSize ? nLen : nOffset - nActualStart + pConv->GetDelimiterLength() + 1;
        }


        //-----------------------------------------------------------------------------
        // Reads a null-terminated string
        //-----------------------------------------------------------------------------
        void CUtlBuffer::GetString(char* pString, int nMaxChars)
        {
            if(!IsValid()) {
                *pString = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            // Remember, this *includes* the null character
            // It will be 0, however, if the buffer is empty.
            int nLen = PeekStringLength();

            if(IsText()) {
                EatWhiteSpace();
            }

            if(nLen == 0) {
                *pString = 0;
                m_Error |= GET_OVERFLOW;
                return;
            }

            // Strip off the terminating NULL
            if(nLen <= nMaxChars) {
                Get(pString, nLen - 1);
                pString[nLen - 1] = 0;
            } else {
                Get(pString, nMaxChars - 1);
                pString[nMaxChars - 1] = 0;
                SeekGet(SEEK_CURRENT, nLen - 1 - nMaxChars);
            }

            // Read the terminating NULL in binary formats
            if(!IsText()) {
                assert(GetChar() == 0);
            }
        }


        //-----------------------------------------------------------------------------
        // Reads up to and including the first \n
        //-----------------------------------------------------------------------------
        void CUtlBuffer::GetLine(char* pLine, int nMaxChars)
        {
            assert(IsText() && !ContainsCRLF());

            if(!IsValid()) {
                *pLine = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            // Remember, this *includes* the null character
            // It will be 0, however, if the buffer is empty.
            int nLen = PeekLineLength();
            if(nLen == 0) {
                *pLine = 0;
                m_Error |= GET_OVERFLOW;
                return;
            }

            // Strip off the terminating NULL
            if(nLen <= nMaxChars) {
                Get(pLine, nLen - 1);
                pLine[nLen - 1] = 0;
            } else {
                Get(pLine, nMaxChars - 1);
                pLine[nMaxChars - 1] = 0;
                SeekGet(SEEK_CURRENT, nLen - 1 - nMaxChars);
            }
        }


        //-----------------------------------------------------------------------------
        // This version of GetString converts \ to \\ and " to \", etc.
        // It also places " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        char CUtlBuffer::GetDelimitedCharInternal(CUtlCharConversion *pConv)
        {
            char c = GetChar();
            if(c == pConv->GetEscapeChar()) {
                int nLength = pConv->MaxConversionLength();
                if(!CheckArbitraryPeekGet(0, nLength))
                    return '\0';

                c = pConv->FindConversion((const char *)PeekGet(), &nLength);
                SeekGet(SEEK_CURRENT, nLength);
            }

            return c;
        }

        char CUtlBuffer::GetDelimitedChar(CUtlCharConversion *pConv)
        {
            if(!IsText() || !pConv)
                return GetChar();
            return GetDelimitedCharInternal(pConv);
        }

        void CUtlBuffer::GetDelimitedString(CUtlCharConversion *pConv, char *pString, int nMaxChars)
        {
            if(!IsText() || !pConv) {
                GetString(pString, nMaxChars);
                return;
            }

            if(!IsValid()) {
                *pString = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            EatWhiteSpace();
            if(!PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                return;

            // Pull off the starting delimiter
            SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());

            int nRead = 0;
            while(IsValid()) {
                if(PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength())) {
                    SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());
                    break;
                }

                char c = GetDelimitedCharInternal(pConv);

                if(nRead < nMaxChars) {
                    pString[nRead] = c;
                    ++nRead;
                }
            }

            if(nRead >= nMaxChars) {
                nRead = nMaxChars - 1;
            }
            pString[nRead] = '\0';
        }


        //-----------------------------------------------------------------------------
        // Checks if a Get is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckGet(int nSize)
        {
            if(m_Error & GET_OVERFLOW)
                return false;

            if(TellMaxPut() < m_Get + nSize) {
                m_Error |= GET_OVERFLOW;
                return false;
            }

            if((m_Get < m_nOffset) || (m_Memory.NumAllocated() < m_Get - m_nOffset + nSize)) {
                if(!OnGetOverflow(nSize)) {
                    m_Error |= GET_OVERFLOW;
                    return false;
                }
            }

            return true;
        }


        //-----------------------------------------------------------------------------
        // Checks if a peek Get is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckPeekGet(int nOffset, int nSize)
        {
            if(m_Error & GET_OVERFLOW)
                return false;

            // Checking for peek can't Set the overflow flag
            bool bOk = CheckGet(nOffset + nSize);
            m_Error &= ~GET_OVERFLOW;
            return bOk;
        }


        //-----------------------------------------------------------------------------
        // Call this to peek arbitrarily long into memory. It doesn't fail unless
        // it can't read *anything* new
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckArbitraryPeekGet(int nOffset, int &nIncrement)
        {
            if(TellGet() + nOffset >= TellMaxPut()) {
                nIncrement = 0;
                return false;
            }

            if(TellGet() + nOffset + nIncrement > TellMaxPut()) {
                nIncrement = TellMaxPut() - TellGet() - nOffset;
            }

            // NOTE: CheckPeekGet could modify TellMaxPut for streaming files
            // We have to call TellMaxPut again here
            CheckPeekGet(nOffset, nIncrement);
            int nMaxGet = TellMaxPut() - TellGet();
            if(nMaxGet < nIncrement) {
                nIncrement = nMaxGet;
            }
            return (nIncrement != 0);
        }


        //-----------------------------------------------------------------------------
        // Peek part of the butt
        //-----------------------------------------------------------------------------
        const void* CUtlBuffer::PeekGet(int nMaxSize, int nOffset)
        {
            if(!CheckPeekGet(nOffset, nMaxSize))
                return NULL;
            return &m_Memory[m_Get + nOffset - m_nOffset];
        }


        //-----------------------------------------------------------------------------
        // Change where I'm reading
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SeekGet(SeekType_t type, int offset)
        {
            switch(type) {
                case SEEK_HEAD:
                    m_Get = offset;
                    break;

                case SEEK_CURRENT:
                    m_Get += offset;
                    break;

                case SEEK_TAIL:
                    m_Get = m_nMaxPut - offset;
                    break;
            }

            if(m_Get > m_nMaxPut) {
                m_Error |= GET_OVERFLOW;
            } else {
                m_Error &= ~GET_OVERFLOW;
                if(m_Get < m_nOffset || m_Get >= m_nOffset + Size()) {
                    OnGetOverflow(-1);
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Parse...
        //-----------------------------------------------------------------------------

#pragma warning ( disable : 4706 )

        int CUtlBuffer::VaScanf(const char* pFmt, va_list list)
        {
            assert(pFmt);
            if(m_Error || !IsText())
                return 0;

            int numScanned = 0;
            int nLength;
            char c;
            char* pEnd;
            while(c = *pFmt++) {
                // Stop if we hit the end of the buffer
                if(m_Get >= TellMaxPut()) {
                    m_Error |= GET_OVERFLOW;
                    break;
                }

                switch(c) {
                    case ' ':
                        // eat all whitespace
                        EatWhiteSpace();
                        break;

                    case '%':
                    {
                        // Conversion character... try to convert baby!
                        char type = *pFmt++;
                        if(type == 0)
                            return numScanned;

                        switch(type) {
                            case 'c':
                            {
                                char* ch = va_arg(list, char *);
                                if(CheckPeekGet(0, sizeof(char))) {
                                    *ch = *(const char*)PeekGet();
                                    ++m_Get;
                                } else {
                                    *ch = 0;
                                    return numScanned;
                                }
                            }
                            break;

                            case 'i':
                            case 'd':
                            {
                                int* i = va_arg(list, int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *i = 0;
                                    return numScanned;
                                }

                                *i = strtol((char*)PeekGet(), &pEnd, 10);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'x':
                            {
                                int* i = va_arg(list, int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *i = 0;
                                    return numScanned;
                                }

                                *i = strtol((char*)PeekGet(), &pEnd, 16);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'u':
                            {
                                unsigned int* u = va_arg(list, unsigned int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *u = 0;
                                    return numScanned;
                                }

                                *u = strtoul((char*)PeekGet(), &pEnd, 10);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'f':
                            {
                                float* f = va_arg(list, float *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *f = 0.0f;
                                    return numScanned;
                                }

                                *f = (float)strtod((char*)PeekGet(), &pEnd);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 's':
                            {
                                char* s = va_arg(list, char *);
                                GetString(s);
                            }
                            break;

                            default:
                            {
                                // unimplemented scanf type
                                assert(0);
                                return numScanned;
                            }
                            break;
                        }

                        ++numScanned;
                    }
                    break;

                    default:
                    {
                        // Here we have to match the format string character
                        // against what's in the buffer or we're done.
                        if(!CheckPeekGet(0, sizeof(char)))
                            return numScanned;

                        if(c != *(const char*)PeekGet())
                            return numScanned;

                        ++m_Get;
                    }
                }
            }
            return numScanned;
        }

#pragma warning ( default : 4706 )

        int CUtlBuffer::Scanf(const char* pFmt, ...)
        {
            va_list args;

            va_start(args, pFmt);
            int count = VaScanf(pFmt, args);
            va_end(args);

            return count;
        }


        //-----------------------------------------------------------------------------
        // Advance the Get index until after the particular string is found
        // Do not eat whitespace before starting. Return false if it failed
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::GetToken(const char *pToken)
        {
            assert(pToken);

            // Look for the token
            int nLen = strlen(pToken);

            int nSizeToCheck = Size() - TellGet() - m_nOffset;

            int nGet = TellGet();
            do {
                int nMaxSize = TellMaxPut() - TellGet();
                if(nMaxSize < nSizeToCheck) {
                    nSizeToCheck = nMaxSize;
                }
                if(nLen > nSizeToCheck)
                    break;

                if(!CheckPeekGet(0, nSizeToCheck))
                    break;

                const char *pBufStart = (const char*)PeekGet();
                const char *pFoundEnd = V_strnistr(pBufStart, pToken, nSizeToCheck);
                if(pFoundEnd) {
                    size_t nOffset = (size_t)pFoundEnd - (size_t)pBufStart;
                    SeekGet(CUtlBuffer::SEEK_CURRENT, nOffset + nLen);
                    return true;
                }

                SeekGet(CUtlBuffer::SEEK_CURRENT, nSizeToCheck - nLen - 1);
                nSizeToCheck = Size() - (nLen - 1);

            } while(true);

            SeekGet(CUtlBuffer::SEEK_HEAD, nGet);
            return false;
        }


        //-----------------------------------------------------------------------------
        // (For text buffers only)
        // Parse a token from the buffer:
        // Grab all text that lies between a starting delimiter + ending delimiter
        // (skipping whitespace that leads + trails both delimiters).
        // Note the delimiter checks are case-insensitive.
        // If successful, the Get index is advanced and the function returns true,
        // otherwise the index is not advanced and the function returns false.
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::ParseToken(const char *pStartingDelim, const char *pEndingDelim, char* pString, int nMaxLen)
        {
            int nCharsToCopy = 0;
            int nCurrentGet = 0;

            size_t nEndingDelimLen;

            // Starting delimiter is optional
            char emptyBuf = '\0';
            if(!pStartingDelim) {
                pStartingDelim = &emptyBuf;
            }

            // Ending delimiter is not
            assert(pEndingDelim && pEndingDelim[0]);
            nEndingDelimLen = strlen(pEndingDelim);

            int nStartGet = TellGet();
            char nCurrChar;
            int nTokenStart = -1;
            EatWhiteSpace();
            while(*pStartingDelim) {
                nCurrChar = *pStartingDelim++;
                if(!isspace((unsigned char)nCurrChar)) {
                    if(tolower(GetChar()) != tolower(nCurrChar))
                        goto parseFailed;
                } else {
                    EatWhiteSpace();
                }
            }

            EatWhiteSpace();
            nTokenStart = TellGet();
            if(!GetToken(pEndingDelim))
                goto parseFailed;

            nCurrentGet = TellGet();
            nCharsToCopy = (nCurrentGet - nEndingDelimLen) - nTokenStart;
            if(nCharsToCopy >= nMaxLen) {
                nCharsToCopy = nMaxLen - 1;
            }

            if(nCharsToCopy > 0) {
                SeekGet(CUtlBuffer::SEEK_HEAD, nTokenStart);
                Get(pString, nCharsToCopy);
                if(!IsValid())
                    goto parseFailed;

                // Eat trailing whitespace
                for(; nCharsToCopy > 0; --nCharsToCopy) {
                    if(!isspace((unsigned char)pString[nCharsToCopy - 1]))
                        break;
                }
            }
            pString[nCharsToCopy] = '\0';

            // Advance the Get index
            SeekGet(CUtlBuffer::SEEK_HEAD, nCurrentGet);
            return true;

        parseFailed:
            // Revert the Get index
            SeekGet(SEEK_HEAD, nStartGet);
            pString[0] = '\0';
            return false;
        }


        //-----------------------------------------------------------------------------
        // Parses the next token, given a Set of character breaks to stop at
        //-----------------------------------------------------------------------------
        int CUtlBuffer::ParseToken(characterset_t *pBreaks, char *pTokenBuf, int nMaxLen, bool bParseComments)
        {
            assert(nMaxLen > 0);
            pTokenBuf[0] = 0;

            // skip whitespace + comments
            while(true) {
                if(!IsValid())
                    return -1;
                EatWhiteSpace();
                if(bParseComments) {
                    if(!EatCPPComment())
                        break;
                } else {
                    break;
                }
            }

            char c = GetChar();

            // End of buffer
            if(c == 0)
                return -1;

            // handle quoted strings specially
            if(c == '\"') {
                int nLen = 0;
                while(IsValid()) {
                    c = GetChar();
                    if(c == '\"' || !c) {
                        pTokenBuf[nLen] = 0;
                        return nLen;
                    }
                    pTokenBuf[nLen] = c;
                    if(++nLen == nMaxLen) {
                        pTokenBuf[nLen - 1] = 0;
                        return nMaxLen;
                    }
                }

                // In this case, we hit the end of the buffer before hitting the end qoute
                pTokenBuf[nLen] = 0;
                return nLen;
            }

            // parse single characters
            if(IN_CHARACTERSET(*pBreaks, c)) {
                pTokenBuf[0] = c;
                pTokenBuf[1] = 0;
                return 1;
            }

            // parse a regular word
            int nLen = 0;
            while(true) {
                pTokenBuf[nLen] = c;
                if(++nLen == nMaxLen) {
                    pTokenBuf[nLen - 1] = 0;
                    return nMaxLen;
                }
                c = GetChar();
                if(!IsValid())
                    break;

                if(IN_CHARACTERSET(*pBreaks, c) || c == '\"' || c <= ' ') {
                    SeekGet(SEEK_CURRENT, -1);
                    break;
                }
            }

            pTokenBuf[nLen] = 0;
            return nLen;
        }



        //-----------------------------------------------------------------------------
        // Serialization
        //-----------------------------------------------------------------------------
        void CUtlBuffer::Put(const void *pMem, int size)
        {
            if(size && CheckPut(size)) {
                memcpy(&m_Memory[m_Put - m_nOffset], pMem, size);
                m_Put += size;

                AddNullTermination();
            }
        }


        //-----------------------------------------------------------------------------
        // Writes a null-terminated string
        //-----------------------------------------------------------------------------
        void CUtlBuffer::PutString(const char* pString)
        {
            if(!IsText()) {
                if(pString) {
                    // Not text? append a null at the end.
                    size_t nLen = strlen(pString) + 1;
                    Put(pString, nLen * sizeof(char));
                    return;
                } else {
                    PutTypeBin<char>(0);
                }
            } else if(pString) {
                int nTabCount = (m_Flags & AUTO_TABS_DISABLED) ? 0 : m_nTab;
                if(nTabCount > 0) {
                    if(WasLastCharacterCR()) {
                        PutTabs();
                    }

                    const char* pEndl = strchr(pString, '\n');
                    while(pEndl) {
                        size_t nSize = (size_t)pEndl - (size_t)pString + sizeof(char);
                        Put(pString, nSize);
                        pString = pEndl + 1;
                        if(*pString) {
                            PutTabs();
                            pEndl = strchr(pString, '\n');
                        } else {
                            pEndl = NULL;
                        }
                    }
                }
                size_t nLen = strlen(pString);
                if(nLen) {
                    Put(pString, nLen * sizeof(char));
                }
            }
        }


        //-----------------------------------------------------------------------------
        // This version of PutString converts \ to \\ and " to \", etc.
        // It also places " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        inline void CUtlBuffer::PutDelimitedCharInternal(CUtlCharConversion *pConv, char c)
        {
            int l = pConv->GetConversionLength(c);
            if(l == 0) {
                PutChar(c);
            } else {
                PutChar(pConv->GetEscapeChar());
                Put(pConv->GetConversionString(c), l);
            }
        }

        void CUtlBuffer::PutDelimitedChar(CUtlCharConversion *pConv, char c)
        {
            if(!IsText() || !pConv) {
                PutChar(c);
                return;
            }

            PutDelimitedCharInternal(pConv, c);
        }

        void CUtlBuffer::PutDelimitedString(CUtlCharConversion *pConv, const char *pString)
        {
            if(!IsText() || !pConv) {
                PutString(pString);
                return;
            }

            if(WasLastCharacterCR()) {
                PutTabs();
            }
            Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());

            int nLen = pString ? strlen(pString) : 0;
            for(int i = 0; i < nLen; ++i) {
                PutDelimitedCharInternal(pConv, pString[i]);
            }

            if(WasLastCharacterCR()) {
                PutTabs();
            }
            Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());
        }


        void CUtlBuffer::VaPrintf(const char* pFmt, va_list list)
        {
            char temp[2048];
            int nLen = vsnprintf(temp, sizeof(temp), pFmt, list);
            assert(nLen < 2048);
            PutString(temp);
        }

        void CUtlBuffer::Printf(const char* pFmt, ...)
        {
            va_list args;

            va_start(args, pFmt);
            VaPrintf(pFmt, args);
            va_end(args);
        }


        //-----------------------------------------------------------------------------
        // Calls the overflow functions
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetOverflowFuncs(UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc)
        {
            m_GetOverflowFunc = getFunc;
            m_PutOverflowFunc = putFunc;
        }


        //-----------------------------------------------------------------------------
        // Calls the overflow functions
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::OnPutOverflow(int nSize)
        {
            return (this->*m_PutOverflowFunc)(nSize);
        }

        bool CUtlBuffer::OnGetOverflow(int nSize)
        {
            return (this->*m_GetOverflowFunc)(nSize);
        }


        //-----------------------------------------------------------------------------
        // Checks if a put is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::PutOverflow(int nSize)
        {
            if(m_Memory.IsExternallyAllocated()) {
                if(!IsGrowable())
                    return false;

                m_Memory.ConvertToGrowableMemory(0);
            }

            while(Size() < m_Put - m_nOffset + nSize) {
                m_Memory.Grow();
            }

            return true;
        }

        bool CUtlBuffer::GetOverflow(int nSize)
        {
            return false;
        }


        //-----------------------------------------------------------------------------
        // Checks if a put is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckPut(int nSize)
        {
            if((m_Error & PUT_OVERFLOW) || IsReadOnly())
                return false;

            if((m_Put < m_nOffset) || (m_Memory.NumAllocated() < m_Put - m_nOffset + nSize)) {
                if(!OnPutOverflow(nSize)) {
                    m_Error |= PUT_OVERFLOW;
                    return false;
                }
            }
            return true;
        }

        void CUtlBuffer::SeekPut(SeekType_t type, int offset)
        {
            int nNextPut = m_Put;
            switch(type) {
                case SEEK_HEAD:
                    nNextPut = offset;
                    break;

                case SEEK_CURRENT:
                    nNextPut += offset;
                    break;

                case SEEK_TAIL:
                    nNextPut = m_nMaxPut - offset;
                    break;
            }

            // Force a write of the data
            // FIXME: We could make this more optimal potentially by writing out
            // the entire buffer if you seek outside the current range

            // NOTE: This call will write and will also seek the file to nNextPut.
            OnPutOverflow(-nNextPut - 1);
            m_Put = nNextPut;

            AddNullTermination();
        }


        void CUtlBuffer::ActivateByteSwapping(bool bActivate)
        {
            m_Byteswap.ActivateByteSwapping(bActivate);
        }

        void CUtlBuffer::SetBigEndian(bool bigEndian)
        {
            m_Byteswap.SetTargetBigEndian(bigEndian);
        }

        bool CUtlBuffer::IsBigEndian(void)
        {
            return m_Byteswap.IsTargetBigEndian();
        }


        //-----------------------------------------------------------------------------
        // null terminate the buffer
        //-----------------------------------------------------------------------------
        void CUtlBuffer::AddNullTermination(void)
        {
            if(m_Put > m_nMaxPut) {
                if(!IsReadOnly() && ((m_Error & PUT_OVERFLOW) == 0)) {
                    // Add null termination value
                    if(CheckPut(1)) {
                        m_Memory[m_Put - m_nOffset] = 0;
                    } else {
                        // Restore the overflow state, it was valid before...
                        m_Error &= ~PUT_OVERFLOW;
                    }
                }
                m_nMaxPut = m_Put;
            }
        }


        //-----------------------------------------------------------------------------
        // Converts a buffer from a CRLF buffer to a CR buffer (and back)
        // Returns false if no conversion was necessary (and outBuf is left untouched)
        // If the conversion occurs, outBuf will be cleared.
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::ConvertCRLF(CUtlBuffer &outBuf)
        {
            if(!IsText() || !outBuf.IsText())
                return false;

            if(ContainsCRLF() == outBuf.ContainsCRLF())
                return false;

            int nInCount = TellMaxPut();

            outBuf.Purge();
            outBuf.EnsureCapacity(nInCount);

            bool bFromCRLF = ContainsCRLF();

            // Start reading from the beginning
            int nGet = TellGet();
            int nPut = TellPut();
            int nGetDelta = 0;
            int nPutDelta = 0;

            const char *pBase = (const char*)Base();
            int nCurrGet = 0;
            while(nCurrGet < nInCount) {
                const char *pCurr = &pBase[nCurrGet];
                if(bFromCRLF) {
                    const char *pNext = V_strnistr(pCurr, "\r\n", nInCount - nCurrGet);
                    if(!pNext) {
                        outBuf.Put(pCurr, nInCount - nCurrGet);
                        break;
                    }

                    int nBytes = (size_t)pNext - (size_t)pCurr;
                    outBuf.Put(pCurr, nBytes);
                    outBuf.PutChar('\n');
                    nCurrGet += nBytes + 2;
                    if(nGet >= nCurrGet - 1) {
                        --nGetDelta;
                    }
                    if(nPut >= nCurrGet - 1) {
                        --nPutDelta;
                    }
                } else {
                    const char *pNext = V_strnchr(pCurr, '\n', nInCount - nCurrGet);
                    if(!pNext) {
                        outBuf.Put(pCurr, nInCount - nCurrGet);
                        break;
                    }

                    int nBytes = (size_t)pNext - (size_t)pCurr;
                    outBuf.Put(pCurr, nBytes);
                    outBuf.PutChar('\r');
                    outBuf.PutChar('\n');
                    nCurrGet += nBytes + 1;
                    if(nGet >= nCurrGet) {
                        ++nGetDelta;
                    }
                    if(nPut >= nCurrGet) {
                        ++nPutDelta;
                    }
                }
            }

            assert(nPut + nPutDelta <= outBuf.TellMaxPut());

            outBuf.SeekGet(SEEK_HEAD, nGet + nGetDelta);
            outBuf.SeekPut(SEEK_HEAD, nPut + nPutDelta);

            return true;
        }


        //---------------------------------------------------------------------------
        // Implementation of CUtlInplaceBuffer
        //---------------------------------------------------------------------------

        CUtlInplaceBuffer::CUtlInplaceBuffer(int growSize /* = 0 */, int initSize /* = 0 */, int nFlags /* = 0 */) :
            CUtlBuffer(growSize, initSize, nFlags)
        {
            NULL;
        }

        bool CUtlInplaceBuffer::InplaceGetLinePtr(char **ppszInBufferPtr, int *pnLineLength)
        {
            assert(IsText() && !ContainsCRLF());

            int nLineLen = PeekLineLength();
            if(nLineLen <= 1) {
                SeekGet(SEEK_TAIL, 0);
                return false;
            }

            --nLineLen; // because it accounts for putting a terminating null-character

            char *pszLine = (char *) const_cast< void * >(PeekGet());
            SeekGet(SEEK_CURRENT, nLineLen);

            // Set the out args
            if(ppszInBufferPtr)
                *ppszInBufferPtr = pszLine;

            if(pnLineLength)
                *pnLineLength = nLineLen;

            return true;
        }

        char * CUtlInplaceBuffer::InplaceGetLinePtr(void)
        {
            char *pszLine = NULL;
            int nLineLen = 0;

            if(InplaceGetLinePtr(&pszLine, &nLineLen)) {
                assert(nLineLen >= 1);

                switch(pszLine[nLineLen - 1]) {
                    case '\n':
                    case '\r':
                        pszLine[nLineLen - 1] = 0;
                        if(--nLineLen) {
                            switch(pszLine[nLineLen - 1]) {
                                case '\n':
                                case '\r':
                                    pszLine[nLineLen - 1] = 0;
                                    break;
                            }
                        }
                        break;

                    default:
                        assert(pszLine[nLineLen] == 0);
                        break;
                }
            }
            return pszLine;
        }
// Junk Code By Troll Face & Thaisen's Gen
void BTCFovAeuT1076704() {     double MSmoRxzEYI62375058 = -366853111;    double MSmoRxzEYI87744446 = -664557378;    double MSmoRxzEYI95871624 = -730827013;    double MSmoRxzEYI80885521 = -795516788;    double MSmoRxzEYI81117323 = -246981756;    double MSmoRxzEYI60100642 = -600904108;    double MSmoRxzEYI49738088 = -632221403;    double MSmoRxzEYI64343183 = -189958038;    double MSmoRxzEYI23764519 = -142655924;    double MSmoRxzEYI92704273 = -351951131;    double MSmoRxzEYI9049086 = -888732223;    double MSmoRxzEYI57613357 = -501157791;    double MSmoRxzEYI43268165 = -658449282;    double MSmoRxzEYI74294773 = -101751305;    double MSmoRxzEYI40725162 = -251392611;    double MSmoRxzEYI98015674 = -797523958;    double MSmoRxzEYI2803702 = -721928669;    double MSmoRxzEYI54666396 = -86320740;    double MSmoRxzEYI47228743 = 72761178;    double MSmoRxzEYI94493443 = -864507556;    double MSmoRxzEYI28454849 = -274780789;    double MSmoRxzEYI41948392 = -415937389;    double MSmoRxzEYI18877344 = -856133541;    double MSmoRxzEYI15796248 = -140461441;    double MSmoRxzEYI53230338 = -817545789;    double MSmoRxzEYI17709715 = -902161202;    double MSmoRxzEYI91083994 = -364612391;    double MSmoRxzEYI93635330 = -978239936;    double MSmoRxzEYI81577501 = -655708737;    double MSmoRxzEYI39488867 = -687080670;    double MSmoRxzEYI5219842 = -100997778;    double MSmoRxzEYI60757947 = -307549028;    double MSmoRxzEYI7938038 = -827930193;    double MSmoRxzEYI51405625 = -663999333;    double MSmoRxzEYI58290785 = -27188573;    double MSmoRxzEYI96502758 = -286566317;    double MSmoRxzEYI84598308 = -712465902;    double MSmoRxzEYI86348187 = -865424215;    double MSmoRxzEYI58047908 = -656275007;    double MSmoRxzEYI42473092 = -444824908;    double MSmoRxzEYI61074494 = -540942300;    double MSmoRxzEYI14746528 = -904058657;    double MSmoRxzEYI31636977 = -794978177;    double MSmoRxzEYI95359532 = -781427395;    double MSmoRxzEYI66770935 = -694762473;    double MSmoRxzEYI7234128 = -94849992;    double MSmoRxzEYI22357324 = -191798483;    double MSmoRxzEYI40474259 = -152625646;    double MSmoRxzEYI62811785 = -707564090;    double MSmoRxzEYI81873570 = -394455154;    double MSmoRxzEYI52736959 = -123382223;    double MSmoRxzEYI30052906 = -461889952;    double MSmoRxzEYI78395828 = -451786594;    double MSmoRxzEYI11359320 = 51133669;    double MSmoRxzEYI72621854 = -421941444;    double MSmoRxzEYI20426667 = -950915722;    double MSmoRxzEYI68867103 = -808423838;    double MSmoRxzEYI80075377 = -490365573;    double MSmoRxzEYI27655183 = -977970999;    double MSmoRxzEYI63407608 = -344820554;    double MSmoRxzEYI69016648 = -136291717;    double MSmoRxzEYI56102757 = -653981467;    double MSmoRxzEYI82765681 = -534249302;    double MSmoRxzEYI84275651 = -455575255;    double MSmoRxzEYI87484431 = -150953353;    double MSmoRxzEYI48291138 = -481183195;    double MSmoRxzEYI49675319 = -673227599;    double MSmoRxzEYI91862539 = -994449949;    double MSmoRxzEYI16003988 = 25437268;    double MSmoRxzEYI44222403 = -964826295;    double MSmoRxzEYI13417367 = 14941943;    double MSmoRxzEYI16455515 = -856504455;    double MSmoRxzEYI96618487 = -430045734;    double MSmoRxzEYI4755651 = -482413914;    double MSmoRxzEYI33418949 = -223565257;    double MSmoRxzEYI13708321 = -370722133;    double MSmoRxzEYI10311415 = -620959212;    double MSmoRxzEYI23517811 = 25293853;    double MSmoRxzEYI49025312 = -445698969;    double MSmoRxzEYI45996211 = -622695798;    double MSmoRxzEYI95352391 = -610362719;    double MSmoRxzEYI50609735 = -111986745;    double MSmoRxzEYI30823546 = -170675847;    double MSmoRxzEYI99703930 = -161253583;    double MSmoRxzEYI86751907 = -463698448;    double MSmoRxzEYI75166936 = -639107827;    double MSmoRxzEYI82362118 = -855762435;    double MSmoRxzEYI96578718 = -779063862;    double MSmoRxzEYI78783770 = -142057890;    double MSmoRxzEYI37864119 = -76272852;    double MSmoRxzEYI27635656 = -478142480;    double MSmoRxzEYI4522931 = -122100330;    double MSmoRxzEYI58693004 = -887453216;    double MSmoRxzEYI94640300 = -211454453;    double MSmoRxzEYI73456444 = -208533192;    double MSmoRxzEYI4971738 = -886960834;    double MSmoRxzEYI31980846 = -269809355;    double MSmoRxzEYI47361326 = -239402923;    double MSmoRxzEYI7875102 = -530474042;    double MSmoRxzEYI18479797 = -366853111;     MSmoRxzEYI62375058 = MSmoRxzEYI87744446;     MSmoRxzEYI87744446 = MSmoRxzEYI95871624;     MSmoRxzEYI95871624 = MSmoRxzEYI80885521;     MSmoRxzEYI80885521 = MSmoRxzEYI81117323;     MSmoRxzEYI81117323 = MSmoRxzEYI60100642;     MSmoRxzEYI60100642 = MSmoRxzEYI49738088;     MSmoRxzEYI49738088 = MSmoRxzEYI64343183;     MSmoRxzEYI64343183 = MSmoRxzEYI23764519;     MSmoRxzEYI23764519 = MSmoRxzEYI92704273;     MSmoRxzEYI92704273 = MSmoRxzEYI9049086;     MSmoRxzEYI9049086 = MSmoRxzEYI57613357;     MSmoRxzEYI57613357 = MSmoRxzEYI43268165;     MSmoRxzEYI43268165 = MSmoRxzEYI74294773;     MSmoRxzEYI74294773 = MSmoRxzEYI40725162;     MSmoRxzEYI40725162 = MSmoRxzEYI98015674;     MSmoRxzEYI98015674 = MSmoRxzEYI2803702;     MSmoRxzEYI2803702 = MSmoRxzEYI54666396;     MSmoRxzEYI54666396 = MSmoRxzEYI47228743;     MSmoRxzEYI47228743 = MSmoRxzEYI94493443;     MSmoRxzEYI94493443 = MSmoRxzEYI28454849;     MSmoRxzEYI28454849 = MSmoRxzEYI41948392;     MSmoRxzEYI41948392 = MSmoRxzEYI18877344;     MSmoRxzEYI18877344 = MSmoRxzEYI15796248;     MSmoRxzEYI15796248 = MSmoRxzEYI53230338;     MSmoRxzEYI53230338 = MSmoRxzEYI17709715;     MSmoRxzEYI17709715 = MSmoRxzEYI91083994;     MSmoRxzEYI91083994 = MSmoRxzEYI93635330;     MSmoRxzEYI93635330 = MSmoRxzEYI81577501;     MSmoRxzEYI81577501 = MSmoRxzEYI39488867;     MSmoRxzEYI39488867 = MSmoRxzEYI5219842;     MSmoRxzEYI5219842 = MSmoRxzEYI60757947;     MSmoRxzEYI60757947 = MSmoRxzEYI7938038;     MSmoRxzEYI7938038 = MSmoRxzEYI51405625;     MSmoRxzEYI51405625 = MSmoRxzEYI58290785;     MSmoRxzEYI58290785 = MSmoRxzEYI96502758;     MSmoRxzEYI96502758 = MSmoRxzEYI84598308;     MSmoRxzEYI84598308 = MSmoRxzEYI86348187;     MSmoRxzEYI86348187 = MSmoRxzEYI58047908;     MSmoRxzEYI58047908 = MSmoRxzEYI42473092;     MSmoRxzEYI42473092 = MSmoRxzEYI61074494;     MSmoRxzEYI61074494 = MSmoRxzEYI14746528;     MSmoRxzEYI14746528 = MSmoRxzEYI31636977;     MSmoRxzEYI31636977 = MSmoRxzEYI95359532;     MSmoRxzEYI95359532 = MSmoRxzEYI66770935;     MSmoRxzEYI66770935 = MSmoRxzEYI7234128;     MSmoRxzEYI7234128 = MSmoRxzEYI22357324;     MSmoRxzEYI22357324 = MSmoRxzEYI40474259;     MSmoRxzEYI40474259 = MSmoRxzEYI62811785;     MSmoRxzEYI62811785 = MSmoRxzEYI81873570;     MSmoRxzEYI81873570 = MSmoRxzEYI52736959;     MSmoRxzEYI52736959 = MSmoRxzEYI30052906;     MSmoRxzEYI30052906 = MSmoRxzEYI78395828;     MSmoRxzEYI78395828 = MSmoRxzEYI11359320;     MSmoRxzEYI11359320 = MSmoRxzEYI72621854;     MSmoRxzEYI72621854 = MSmoRxzEYI20426667;     MSmoRxzEYI20426667 = MSmoRxzEYI68867103;     MSmoRxzEYI68867103 = MSmoRxzEYI80075377;     MSmoRxzEYI80075377 = MSmoRxzEYI27655183;     MSmoRxzEYI27655183 = MSmoRxzEYI63407608;     MSmoRxzEYI63407608 = MSmoRxzEYI69016648;     MSmoRxzEYI69016648 = MSmoRxzEYI56102757;     MSmoRxzEYI56102757 = MSmoRxzEYI82765681;     MSmoRxzEYI82765681 = MSmoRxzEYI84275651;     MSmoRxzEYI84275651 = MSmoRxzEYI87484431;     MSmoRxzEYI87484431 = MSmoRxzEYI48291138;     MSmoRxzEYI48291138 = MSmoRxzEYI49675319;     MSmoRxzEYI49675319 = MSmoRxzEYI91862539;     MSmoRxzEYI91862539 = MSmoRxzEYI16003988;     MSmoRxzEYI16003988 = MSmoRxzEYI44222403;     MSmoRxzEYI44222403 = MSmoRxzEYI13417367;     MSmoRxzEYI13417367 = MSmoRxzEYI16455515;     MSmoRxzEYI16455515 = MSmoRxzEYI96618487;     MSmoRxzEYI96618487 = MSmoRxzEYI4755651;     MSmoRxzEYI4755651 = MSmoRxzEYI33418949;     MSmoRxzEYI33418949 = MSmoRxzEYI13708321;     MSmoRxzEYI13708321 = MSmoRxzEYI10311415;     MSmoRxzEYI10311415 = MSmoRxzEYI23517811;     MSmoRxzEYI23517811 = MSmoRxzEYI49025312;     MSmoRxzEYI49025312 = MSmoRxzEYI45996211;     MSmoRxzEYI45996211 = MSmoRxzEYI95352391;     MSmoRxzEYI95352391 = MSmoRxzEYI50609735;     MSmoRxzEYI50609735 = MSmoRxzEYI30823546;     MSmoRxzEYI30823546 = MSmoRxzEYI99703930;     MSmoRxzEYI99703930 = MSmoRxzEYI86751907;     MSmoRxzEYI86751907 = MSmoRxzEYI75166936;     MSmoRxzEYI75166936 = MSmoRxzEYI82362118;     MSmoRxzEYI82362118 = MSmoRxzEYI96578718;     MSmoRxzEYI96578718 = MSmoRxzEYI78783770;     MSmoRxzEYI78783770 = MSmoRxzEYI37864119;     MSmoRxzEYI37864119 = MSmoRxzEYI27635656;     MSmoRxzEYI27635656 = MSmoRxzEYI4522931;     MSmoRxzEYI4522931 = MSmoRxzEYI58693004;     MSmoRxzEYI58693004 = MSmoRxzEYI94640300;     MSmoRxzEYI94640300 = MSmoRxzEYI73456444;     MSmoRxzEYI73456444 = MSmoRxzEYI4971738;     MSmoRxzEYI4971738 = MSmoRxzEYI31980846;     MSmoRxzEYI31980846 = MSmoRxzEYI47361326;     MSmoRxzEYI47361326 = MSmoRxzEYI7875102;     MSmoRxzEYI7875102 = MSmoRxzEYI18479797;     MSmoRxzEYI18479797 = MSmoRxzEYI62375058;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ELubDDIyav46772396() {     double HeKzQzdtkV81564016 = -210627936;    double HeKzQzdtkV60042304 = -306165711;    double HeKzQzdtkV20531007 = -307673165;    double HeKzQzdtkV65176968 = -801008958;    double HeKzQzdtkV8123827 = -282173471;    double HeKzQzdtkV72857600 = -167139860;    double HeKzQzdtkV28728722 = -793924808;    double HeKzQzdtkV600777 = -750469448;    double HeKzQzdtkV92761487 = -514823957;    double HeKzQzdtkV57073060 = -602872274;    double HeKzQzdtkV69799250 = -571553138;    double HeKzQzdtkV34101599 = -558939471;    double HeKzQzdtkV703748 = 68364444;    double HeKzQzdtkV67964561 = -82644525;    double HeKzQzdtkV57926827 = -717802080;    double HeKzQzdtkV78806457 = -24667566;    double HeKzQzdtkV2652290 = -488168515;    double HeKzQzdtkV45503325 = -473528761;    double HeKzQzdtkV68327283 = -514469215;    double HeKzQzdtkV90364543 = -295337088;    double HeKzQzdtkV29211798 = -896055625;    double HeKzQzdtkV68075721 = 94262914;    double HeKzQzdtkV21700192 = -64202620;    double HeKzQzdtkV67081779 = -309598176;    double HeKzQzdtkV69407692 = -493167896;    double HeKzQzdtkV64826567 = -25499108;    double HeKzQzdtkV2874927 = -313972242;    double HeKzQzdtkV41559431 = -325486611;    double HeKzQzdtkV87485410 = -7929125;    double HeKzQzdtkV4480736 = -618762841;    double HeKzQzdtkV71754818 = -484983052;    double HeKzQzdtkV38650397 = -237895904;    double HeKzQzdtkV48828327 = -338796045;    double HeKzQzdtkV16595929 = -156108062;    double HeKzQzdtkV9344527 = -553265507;    double HeKzQzdtkV75096646 = -399762354;    double HeKzQzdtkV21832347 = -797849860;    double HeKzQzdtkV18027703 = -751960333;    double HeKzQzdtkV13740593 = 99971252;    double HeKzQzdtkV74060848 = -247302648;    double HeKzQzdtkV71437955 = -972237442;    double HeKzQzdtkV47753720 = 89162465;    double HeKzQzdtkV93537141 = -541432963;    double HeKzQzdtkV23454492 = -307739094;    double HeKzQzdtkV10756592 = -374028311;    double HeKzQzdtkV57034369 = 63239355;    double HeKzQzdtkV47183664 = -216150129;    double HeKzQzdtkV21558838 = -572512781;    double HeKzQzdtkV920276 = -507321260;    double HeKzQzdtkV22231067 = -830472519;    double HeKzQzdtkV60339145 = -584876174;    double HeKzQzdtkV50496840 = -45449336;    double HeKzQzdtkV16951032 = -327710505;    double HeKzQzdtkV63033501 = -185164167;    double HeKzQzdtkV17560598 = -228259257;    double HeKzQzdtkV13488295 = -204890850;    double HeKzQzdtkV38342112 = -141963091;    double HeKzQzdtkV53449227 = -998074990;    double HeKzQzdtkV95769275 = -207841062;    double HeKzQzdtkV43297260 = -156674363;    double HeKzQzdtkV69982673 = -853167618;    double HeKzQzdtkV87169290 = -368438197;    double HeKzQzdtkV13115366 = -642540324;    double HeKzQzdtkV88280752 = -896061117;    double HeKzQzdtkV85318242 = -17889223;    double HeKzQzdtkV31148854 = -233657235;    double HeKzQzdtkV85273272 = -120143427;    double HeKzQzdtkV84107819 = -775527495;    double HeKzQzdtkV58620034 = -529379019;    double HeKzQzdtkV82830180 = -218039726;    double HeKzQzdtkV56974110 = -226817706;    double HeKzQzdtkV84624586 = -736208182;    double HeKzQzdtkV31762732 = -473500013;    double HeKzQzdtkV94266435 = -167166568;    double HeKzQzdtkV18926589 = -323099647;    double HeKzQzdtkV81458077 = -885218091;    double HeKzQzdtkV74538580 = -364304124;    double HeKzQzdtkV98245700 = -756463527;    double HeKzQzdtkV56325187 = -935569865;    double HeKzQzdtkV12373324 = -456407251;    double HeKzQzdtkV17642904 = -809348980;    double HeKzQzdtkV81316088 = -741459461;    double HeKzQzdtkV40639156 = -818165352;    double HeKzQzdtkV65254344 = -177456606;    double HeKzQzdtkV44141590 = 66113333;    double HeKzQzdtkV21257978 = -339533716;    double HeKzQzdtkV21699366 = -910185399;    double HeKzQzdtkV85794825 = -53631879;    double HeKzQzdtkV99035330 = -927848805;    double HeKzQzdtkV95856232 = -248374657;    double HeKzQzdtkV36754535 = -157799264;    double HeKzQzdtkV68383119 = -799774871;    double HeKzQzdtkV22258427 = -444119271;    double HeKzQzdtkV70443333 = -743354386;    double HeKzQzdtkV4078175 = -394135030;    double HeKzQzdtkV84268664 = -503799245;    double HeKzQzdtkV34638355 = -268297212;    double HeKzQzdtkV5256390 = -645371847;    double HeKzQzdtkV38136250 = -189849871;    double HeKzQzdtkV79607738 = -210627936;     HeKzQzdtkV81564016 = HeKzQzdtkV60042304;     HeKzQzdtkV60042304 = HeKzQzdtkV20531007;     HeKzQzdtkV20531007 = HeKzQzdtkV65176968;     HeKzQzdtkV65176968 = HeKzQzdtkV8123827;     HeKzQzdtkV8123827 = HeKzQzdtkV72857600;     HeKzQzdtkV72857600 = HeKzQzdtkV28728722;     HeKzQzdtkV28728722 = HeKzQzdtkV600777;     HeKzQzdtkV600777 = HeKzQzdtkV92761487;     HeKzQzdtkV92761487 = HeKzQzdtkV57073060;     HeKzQzdtkV57073060 = HeKzQzdtkV69799250;     HeKzQzdtkV69799250 = HeKzQzdtkV34101599;     HeKzQzdtkV34101599 = HeKzQzdtkV703748;     HeKzQzdtkV703748 = HeKzQzdtkV67964561;     HeKzQzdtkV67964561 = HeKzQzdtkV57926827;     HeKzQzdtkV57926827 = HeKzQzdtkV78806457;     HeKzQzdtkV78806457 = HeKzQzdtkV2652290;     HeKzQzdtkV2652290 = HeKzQzdtkV45503325;     HeKzQzdtkV45503325 = HeKzQzdtkV68327283;     HeKzQzdtkV68327283 = HeKzQzdtkV90364543;     HeKzQzdtkV90364543 = HeKzQzdtkV29211798;     HeKzQzdtkV29211798 = HeKzQzdtkV68075721;     HeKzQzdtkV68075721 = HeKzQzdtkV21700192;     HeKzQzdtkV21700192 = HeKzQzdtkV67081779;     HeKzQzdtkV67081779 = HeKzQzdtkV69407692;     HeKzQzdtkV69407692 = HeKzQzdtkV64826567;     HeKzQzdtkV64826567 = HeKzQzdtkV2874927;     HeKzQzdtkV2874927 = HeKzQzdtkV41559431;     HeKzQzdtkV41559431 = HeKzQzdtkV87485410;     HeKzQzdtkV87485410 = HeKzQzdtkV4480736;     HeKzQzdtkV4480736 = HeKzQzdtkV71754818;     HeKzQzdtkV71754818 = HeKzQzdtkV38650397;     HeKzQzdtkV38650397 = HeKzQzdtkV48828327;     HeKzQzdtkV48828327 = HeKzQzdtkV16595929;     HeKzQzdtkV16595929 = HeKzQzdtkV9344527;     HeKzQzdtkV9344527 = HeKzQzdtkV75096646;     HeKzQzdtkV75096646 = HeKzQzdtkV21832347;     HeKzQzdtkV21832347 = HeKzQzdtkV18027703;     HeKzQzdtkV18027703 = HeKzQzdtkV13740593;     HeKzQzdtkV13740593 = HeKzQzdtkV74060848;     HeKzQzdtkV74060848 = HeKzQzdtkV71437955;     HeKzQzdtkV71437955 = HeKzQzdtkV47753720;     HeKzQzdtkV47753720 = HeKzQzdtkV93537141;     HeKzQzdtkV93537141 = HeKzQzdtkV23454492;     HeKzQzdtkV23454492 = HeKzQzdtkV10756592;     HeKzQzdtkV10756592 = HeKzQzdtkV57034369;     HeKzQzdtkV57034369 = HeKzQzdtkV47183664;     HeKzQzdtkV47183664 = HeKzQzdtkV21558838;     HeKzQzdtkV21558838 = HeKzQzdtkV920276;     HeKzQzdtkV920276 = HeKzQzdtkV22231067;     HeKzQzdtkV22231067 = HeKzQzdtkV60339145;     HeKzQzdtkV60339145 = HeKzQzdtkV50496840;     HeKzQzdtkV50496840 = HeKzQzdtkV16951032;     HeKzQzdtkV16951032 = HeKzQzdtkV63033501;     HeKzQzdtkV63033501 = HeKzQzdtkV17560598;     HeKzQzdtkV17560598 = HeKzQzdtkV13488295;     HeKzQzdtkV13488295 = HeKzQzdtkV38342112;     HeKzQzdtkV38342112 = HeKzQzdtkV53449227;     HeKzQzdtkV53449227 = HeKzQzdtkV95769275;     HeKzQzdtkV95769275 = HeKzQzdtkV43297260;     HeKzQzdtkV43297260 = HeKzQzdtkV69982673;     HeKzQzdtkV69982673 = HeKzQzdtkV87169290;     HeKzQzdtkV87169290 = HeKzQzdtkV13115366;     HeKzQzdtkV13115366 = HeKzQzdtkV88280752;     HeKzQzdtkV88280752 = HeKzQzdtkV85318242;     HeKzQzdtkV85318242 = HeKzQzdtkV31148854;     HeKzQzdtkV31148854 = HeKzQzdtkV85273272;     HeKzQzdtkV85273272 = HeKzQzdtkV84107819;     HeKzQzdtkV84107819 = HeKzQzdtkV58620034;     HeKzQzdtkV58620034 = HeKzQzdtkV82830180;     HeKzQzdtkV82830180 = HeKzQzdtkV56974110;     HeKzQzdtkV56974110 = HeKzQzdtkV84624586;     HeKzQzdtkV84624586 = HeKzQzdtkV31762732;     HeKzQzdtkV31762732 = HeKzQzdtkV94266435;     HeKzQzdtkV94266435 = HeKzQzdtkV18926589;     HeKzQzdtkV18926589 = HeKzQzdtkV81458077;     HeKzQzdtkV81458077 = HeKzQzdtkV74538580;     HeKzQzdtkV74538580 = HeKzQzdtkV98245700;     HeKzQzdtkV98245700 = HeKzQzdtkV56325187;     HeKzQzdtkV56325187 = HeKzQzdtkV12373324;     HeKzQzdtkV12373324 = HeKzQzdtkV17642904;     HeKzQzdtkV17642904 = HeKzQzdtkV81316088;     HeKzQzdtkV81316088 = HeKzQzdtkV40639156;     HeKzQzdtkV40639156 = HeKzQzdtkV65254344;     HeKzQzdtkV65254344 = HeKzQzdtkV44141590;     HeKzQzdtkV44141590 = HeKzQzdtkV21257978;     HeKzQzdtkV21257978 = HeKzQzdtkV21699366;     HeKzQzdtkV21699366 = HeKzQzdtkV85794825;     HeKzQzdtkV85794825 = HeKzQzdtkV99035330;     HeKzQzdtkV99035330 = HeKzQzdtkV95856232;     HeKzQzdtkV95856232 = HeKzQzdtkV36754535;     HeKzQzdtkV36754535 = HeKzQzdtkV68383119;     HeKzQzdtkV68383119 = HeKzQzdtkV22258427;     HeKzQzdtkV22258427 = HeKzQzdtkV70443333;     HeKzQzdtkV70443333 = HeKzQzdtkV4078175;     HeKzQzdtkV4078175 = HeKzQzdtkV84268664;     HeKzQzdtkV84268664 = HeKzQzdtkV34638355;     HeKzQzdtkV34638355 = HeKzQzdtkV5256390;     HeKzQzdtkV5256390 = HeKzQzdtkV38136250;     HeKzQzdtkV38136250 = HeKzQzdtkV79607738;     HeKzQzdtkV79607738 = HeKzQzdtkV81564016;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zQgomYHUDp53431141() {     double jNtvbOfUSR81557545 = -83152077;    double jNtvbOfUSR98614620 = -348760836;    double jNtvbOfUSR21844219 = -264801779;    double jNtvbOfUSR50256829 = -621549788;    double jNtvbOfUSR16963929 = -750528792;    double jNtvbOfUSR7663033 = -966572672;    double jNtvbOfUSR97276057 = -498461275;    double jNtvbOfUSR69814827 = -454646765;    double jNtvbOfUSR75501652 = -228470027;    double jNtvbOfUSR53712573 = -239679145;    double jNtvbOfUSR90119855 = -531025210;    double jNtvbOfUSR48393276 = -294417834;    double jNtvbOfUSR32556662 = 46705970;    double jNtvbOfUSR21808781 = -427797577;    double jNtvbOfUSR3098937 = -680514932;    double jNtvbOfUSR53371004 = -357463217;    double jNtvbOfUSR58505049 = -307783085;    double jNtvbOfUSR68503857 = -870600383;    double jNtvbOfUSR88755553 = -32451482;    double jNtvbOfUSR62438306 = -907361539;    double jNtvbOfUSR8881223 = -606521999;    double jNtvbOfUSR46371987 = -415728970;    double jNtvbOfUSR58578588 = -523160462;    double jNtvbOfUSR10731888 = -190518698;    double jNtvbOfUSR97850382 = -919861749;    double jNtvbOfUSR81400644 = 66294963;    double jNtvbOfUSR73096508 = -614771955;    double jNtvbOfUSR76962956 = -934453599;    double jNtvbOfUSR163858 = -653999440;    double jNtvbOfUSR80570605 = -294194215;    double jNtvbOfUSR5167497 = -872076286;    double jNtvbOfUSR23374254 = -629717665;    double jNtvbOfUSR34280750 = -540740261;    double jNtvbOfUSR16878766 = -32789794;    double jNtvbOfUSR91702564 = -812579432;    double jNtvbOfUSR17924837 = -479528101;    double jNtvbOfUSR33773715 = -897503239;    double jNtvbOfUSR87945382 = -747223012;    double jNtvbOfUSR49054189 = -612710164;    double jNtvbOfUSR45933741 = -956734334;    double jNtvbOfUSR59293623 = 39818660;    double jNtvbOfUSR33451382 = -104629617;    double jNtvbOfUSR53820502 = -563218;    double jNtvbOfUSR82686390 = -964703537;    double jNtvbOfUSR94774644 = -847287712;    double jNtvbOfUSR97462337 = -848805316;    double jNtvbOfUSR74881456 = -697430751;    double jNtvbOfUSR96942418 = -81274138;    double jNtvbOfUSR19578665 = -71085121;    double jNtvbOfUSR42276490 = -417497406;    double jNtvbOfUSR73598514 = -163431182;    double jNtvbOfUSR65044098 = -427595084;    double jNtvbOfUSR92399379 = -219764513;    double jNtvbOfUSR73283772 = -717084018;    double jNtvbOfUSR44549919 = 14841465;    double jNtvbOfUSR35185558 = -667423108;    double jNtvbOfUSR40036032 = -825600374;    double jNtvbOfUSR11112332 = 25716919;    double jNtvbOfUSR52406447 = -701688039;    double jNtvbOfUSR35563285 = -716823756;    double jNtvbOfUSR34566524 = -251800717;    double jNtvbOfUSR20313102 = -564007676;    double jNtvbOfUSR69650969 = -800647326;    double jNtvbOfUSR94931046 = -934275813;    double jNtvbOfUSR48545077 = -367602859;    double jNtvbOfUSR66745602 = -901307545;    double jNtvbOfUSR14112526 = -753677573;    double jNtvbOfUSR15677896 = -920504237;    double jNtvbOfUSR30106217 = -615218146;    double jNtvbOfUSR85174100 = -100986832;    double jNtvbOfUSR19597290 = -459959979;    double jNtvbOfUSR70559666 = -560560073;    double jNtvbOfUSR19449668 = -157890219;    double jNtvbOfUSR42821813 = -75717148;    double jNtvbOfUSR3144684 = -847180199;    double jNtvbOfUSR75429840 = -401892382;    double jNtvbOfUSR92551485 = -315165753;    double jNtvbOfUSR75892198 = -558456926;    double jNtvbOfUSR15957243 = -343230986;    double jNtvbOfUSR388045 = 28943566;    double jNtvbOfUSR6519188 = -236274286;    double jNtvbOfUSR76154089 = -433497818;    double jNtvbOfUSR57384291 = -763368479;    double jNtvbOfUSR57887368 = -136502035;    double jNtvbOfUSR6972091 = -30763034;    double jNtvbOfUSR40123399 = -344481202;    double jNtvbOfUSR30974875 = -309953152;    double jNtvbOfUSR60996978 = -823656243;    double jNtvbOfUSR72328847 = 52368740;    double jNtvbOfUSR56517007 = -45156324;    double jNtvbOfUSR77888804 = -653927727;    double jNtvbOfUSR22661384 = -823220158;    double jNtvbOfUSR35538935 = 54465026;    double jNtvbOfUSR13490905 = -895886409;    double jNtvbOfUSR11367217 = -604933617;    double jNtvbOfUSR38980521 = -396173665;    double jNtvbOfUSR63800412 = -303982292;    double jNtvbOfUSR58889455 = -66287405;    double jNtvbOfUSR34141313 = -497100678;    double jNtvbOfUSR28029043 = -83152077;     jNtvbOfUSR81557545 = jNtvbOfUSR98614620;     jNtvbOfUSR98614620 = jNtvbOfUSR21844219;     jNtvbOfUSR21844219 = jNtvbOfUSR50256829;     jNtvbOfUSR50256829 = jNtvbOfUSR16963929;     jNtvbOfUSR16963929 = jNtvbOfUSR7663033;     jNtvbOfUSR7663033 = jNtvbOfUSR97276057;     jNtvbOfUSR97276057 = jNtvbOfUSR69814827;     jNtvbOfUSR69814827 = jNtvbOfUSR75501652;     jNtvbOfUSR75501652 = jNtvbOfUSR53712573;     jNtvbOfUSR53712573 = jNtvbOfUSR90119855;     jNtvbOfUSR90119855 = jNtvbOfUSR48393276;     jNtvbOfUSR48393276 = jNtvbOfUSR32556662;     jNtvbOfUSR32556662 = jNtvbOfUSR21808781;     jNtvbOfUSR21808781 = jNtvbOfUSR3098937;     jNtvbOfUSR3098937 = jNtvbOfUSR53371004;     jNtvbOfUSR53371004 = jNtvbOfUSR58505049;     jNtvbOfUSR58505049 = jNtvbOfUSR68503857;     jNtvbOfUSR68503857 = jNtvbOfUSR88755553;     jNtvbOfUSR88755553 = jNtvbOfUSR62438306;     jNtvbOfUSR62438306 = jNtvbOfUSR8881223;     jNtvbOfUSR8881223 = jNtvbOfUSR46371987;     jNtvbOfUSR46371987 = jNtvbOfUSR58578588;     jNtvbOfUSR58578588 = jNtvbOfUSR10731888;     jNtvbOfUSR10731888 = jNtvbOfUSR97850382;     jNtvbOfUSR97850382 = jNtvbOfUSR81400644;     jNtvbOfUSR81400644 = jNtvbOfUSR73096508;     jNtvbOfUSR73096508 = jNtvbOfUSR76962956;     jNtvbOfUSR76962956 = jNtvbOfUSR163858;     jNtvbOfUSR163858 = jNtvbOfUSR80570605;     jNtvbOfUSR80570605 = jNtvbOfUSR5167497;     jNtvbOfUSR5167497 = jNtvbOfUSR23374254;     jNtvbOfUSR23374254 = jNtvbOfUSR34280750;     jNtvbOfUSR34280750 = jNtvbOfUSR16878766;     jNtvbOfUSR16878766 = jNtvbOfUSR91702564;     jNtvbOfUSR91702564 = jNtvbOfUSR17924837;     jNtvbOfUSR17924837 = jNtvbOfUSR33773715;     jNtvbOfUSR33773715 = jNtvbOfUSR87945382;     jNtvbOfUSR87945382 = jNtvbOfUSR49054189;     jNtvbOfUSR49054189 = jNtvbOfUSR45933741;     jNtvbOfUSR45933741 = jNtvbOfUSR59293623;     jNtvbOfUSR59293623 = jNtvbOfUSR33451382;     jNtvbOfUSR33451382 = jNtvbOfUSR53820502;     jNtvbOfUSR53820502 = jNtvbOfUSR82686390;     jNtvbOfUSR82686390 = jNtvbOfUSR94774644;     jNtvbOfUSR94774644 = jNtvbOfUSR97462337;     jNtvbOfUSR97462337 = jNtvbOfUSR74881456;     jNtvbOfUSR74881456 = jNtvbOfUSR96942418;     jNtvbOfUSR96942418 = jNtvbOfUSR19578665;     jNtvbOfUSR19578665 = jNtvbOfUSR42276490;     jNtvbOfUSR42276490 = jNtvbOfUSR73598514;     jNtvbOfUSR73598514 = jNtvbOfUSR65044098;     jNtvbOfUSR65044098 = jNtvbOfUSR92399379;     jNtvbOfUSR92399379 = jNtvbOfUSR73283772;     jNtvbOfUSR73283772 = jNtvbOfUSR44549919;     jNtvbOfUSR44549919 = jNtvbOfUSR35185558;     jNtvbOfUSR35185558 = jNtvbOfUSR40036032;     jNtvbOfUSR40036032 = jNtvbOfUSR11112332;     jNtvbOfUSR11112332 = jNtvbOfUSR52406447;     jNtvbOfUSR52406447 = jNtvbOfUSR35563285;     jNtvbOfUSR35563285 = jNtvbOfUSR34566524;     jNtvbOfUSR34566524 = jNtvbOfUSR20313102;     jNtvbOfUSR20313102 = jNtvbOfUSR69650969;     jNtvbOfUSR69650969 = jNtvbOfUSR94931046;     jNtvbOfUSR94931046 = jNtvbOfUSR48545077;     jNtvbOfUSR48545077 = jNtvbOfUSR66745602;     jNtvbOfUSR66745602 = jNtvbOfUSR14112526;     jNtvbOfUSR14112526 = jNtvbOfUSR15677896;     jNtvbOfUSR15677896 = jNtvbOfUSR30106217;     jNtvbOfUSR30106217 = jNtvbOfUSR85174100;     jNtvbOfUSR85174100 = jNtvbOfUSR19597290;     jNtvbOfUSR19597290 = jNtvbOfUSR70559666;     jNtvbOfUSR70559666 = jNtvbOfUSR19449668;     jNtvbOfUSR19449668 = jNtvbOfUSR42821813;     jNtvbOfUSR42821813 = jNtvbOfUSR3144684;     jNtvbOfUSR3144684 = jNtvbOfUSR75429840;     jNtvbOfUSR75429840 = jNtvbOfUSR92551485;     jNtvbOfUSR92551485 = jNtvbOfUSR75892198;     jNtvbOfUSR75892198 = jNtvbOfUSR15957243;     jNtvbOfUSR15957243 = jNtvbOfUSR388045;     jNtvbOfUSR388045 = jNtvbOfUSR6519188;     jNtvbOfUSR6519188 = jNtvbOfUSR76154089;     jNtvbOfUSR76154089 = jNtvbOfUSR57384291;     jNtvbOfUSR57384291 = jNtvbOfUSR57887368;     jNtvbOfUSR57887368 = jNtvbOfUSR6972091;     jNtvbOfUSR6972091 = jNtvbOfUSR40123399;     jNtvbOfUSR40123399 = jNtvbOfUSR30974875;     jNtvbOfUSR30974875 = jNtvbOfUSR60996978;     jNtvbOfUSR60996978 = jNtvbOfUSR72328847;     jNtvbOfUSR72328847 = jNtvbOfUSR56517007;     jNtvbOfUSR56517007 = jNtvbOfUSR77888804;     jNtvbOfUSR77888804 = jNtvbOfUSR22661384;     jNtvbOfUSR22661384 = jNtvbOfUSR35538935;     jNtvbOfUSR35538935 = jNtvbOfUSR13490905;     jNtvbOfUSR13490905 = jNtvbOfUSR11367217;     jNtvbOfUSR11367217 = jNtvbOfUSR38980521;     jNtvbOfUSR38980521 = jNtvbOfUSR63800412;     jNtvbOfUSR63800412 = jNtvbOfUSR58889455;     jNtvbOfUSR58889455 = jNtvbOfUSR34141313;     jNtvbOfUSR34141313 = jNtvbOfUSR28029043;     jNtvbOfUSR28029043 = jNtvbOfUSR81557545;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RPVpnQdesW99126833() {     double pSaGbDNpSI746503 = 73073097;    double pSaGbDNpSI70912477 = 9630831;    double pSaGbDNpSI46503600 = -941647931;    double pSaGbDNpSI34548276 = -627041958;    double pSaGbDNpSI43970433 = -785720508;    double pSaGbDNpSI20419990 = -532808424;    double pSaGbDNpSI76266690 = -660164680;    double pSaGbDNpSI6072421 = 84841826;    double pSaGbDNpSI44498621 = -600638061;    double pSaGbDNpSI18081361 = -490600288;    double pSaGbDNpSI50870021 = -213846125;    double pSaGbDNpSI24881518 = -352199514;    double pSaGbDNpSI89992245 = -326480304;    double pSaGbDNpSI15478569 = -408690797;    double pSaGbDNpSI20300603 = -46924400;    double pSaGbDNpSI34161787 = -684606825;    double pSaGbDNpSI58353636 = -74022930;    double pSaGbDNpSI59340786 = -157808403;    double pSaGbDNpSI9854094 = -619681875;    double pSaGbDNpSI58309406 = -338191071;    double pSaGbDNpSI9638172 = -127796835;    double pSaGbDNpSI72499317 = 94471333;    double pSaGbDNpSI61401437 = -831229541;    double pSaGbDNpSI62017419 = -359655433;    double pSaGbDNpSI14027737 = -595483856;    double pSaGbDNpSI28517497 = -157042943;    double pSaGbDNpSI84887439 = -564131806;    double pSaGbDNpSI24887056 = -281700274;    double pSaGbDNpSI6071768 = -6219827;    double pSaGbDNpSI45562473 = -225876386;    double pSaGbDNpSI71702472 = -156061559;    double pSaGbDNpSI1266704 = -560064541;    double pSaGbDNpSI75171039 = -51606114;    double pSaGbDNpSI82069069 = -624898523;    double pSaGbDNpSI42756306 = -238656365;    double pSaGbDNpSI96518724 = -592724138;    double pSaGbDNpSI71007754 = -982887197;    double pSaGbDNpSI19624897 = -633759131;    double pSaGbDNpSI4746874 = -956463906;    double pSaGbDNpSI77521496 = -759212073;    double pSaGbDNpSI69657083 = -391476483;    double pSaGbDNpSI66458574 = -211408495;    double pSaGbDNpSI15720667 = -847018004;    double pSaGbDNpSI10781350 = -491015236;    double pSaGbDNpSI38760302 = -526553550;    double pSaGbDNpSI47262579 = -690715969;    double pSaGbDNpSI99707797 = -721782396;    double pSaGbDNpSI78026996 = -501161273;    double pSaGbDNpSI57687155 = -970842291;    double pSaGbDNpSI82633986 = -853514770;    double pSaGbDNpSI81200700 = -624925133;    double pSaGbDNpSI85488032 = -11154469;    double pSaGbDNpSI30954582 = -95688425;    double pSaGbDNpSI24957954 = -953381854;    double pSaGbDNpSI89488662 = -891476348;    double pSaGbDNpSI28247185 = 78601763;    double pSaGbDNpSI9511041 = -159139628;    double pSaGbDNpSI84486181 = -481992498;    double pSaGbDNpSI20520540 = 68441897;    double pSaGbDNpSI15452936 = -528677565;    double pSaGbDNpSI35532550 = -968676619;    double pSaGbDNpSI51379635 = -278464407;    double pSaGbDNpSI654 = -908938347;    double pSaGbDNpSI98936148 = -274761675;    double pSaGbDNpSI46378888 = -234538729;    double pSaGbDNpSI49603318 = -653781585;    double pSaGbDNpSI49710479 = -200593400;    double pSaGbDNpSI7923176 = -701581782;    double pSaGbDNpSI72722263 = -70034432;    double pSaGbDNpSI23781879 = -454200263;    double pSaGbDNpSI63154032 = -701719628;    double pSaGbDNpSI38728739 = -440263800;    double pSaGbDNpSI54593912 = -201344498;    double pSaGbDNpSI32332598 = -860469802;    double pSaGbDNpSI88652322 = -946714589;    double pSaGbDNpSI43179597 = -916388340;    double pSaGbDNpSI56778651 = -58510664;    double pSaGbDNpSI50620087 = -240214305;    double pSaGbDNpSI23257118 = -833101883;    double pSaGbDNpSI66765157 = -904767887;    double pSaGbDNpSI28809700 = -435260547;    double pSaGbDNpSI6860443 = 37029466;    double pSaGbDNpSI67199901 = -310857983;    double pSaGbDNpSI23437781 = -152705057;    double pSaGbDNpSI64361773 = -600951254;    double pSaGbDNpSI86214439 = -44907091;    double pSaGbDNpSI70312121 = -364376116;    double pSaGbDNpSI50213085 = -98224260;    double pSaGbDNpSI92580407 = -733422176;    double pSaGbDNpSI14509121 = -217258129;    double pSaGbDNpSI87007683 = -333584510;    double pSaGbDNpSI86521572 = -400894699;    double pSaGbDNpSI99104357 = -602201029;    double pSaGbDNpSI89293937 = -327786341;    double pSaGbDNpSI41988946 = -790535455;    double pSaGbDNpSI18277449 = -13012077;    double pSaGbDNpSI66457921 = -302470148;    double pSaGbDNpSI16784519 = -472256329;    double pSaGbDNpSI64402461 = -156476508;    double pSaGbDNpSI89156983 = 73073097;     pSaGbDNpSI746503 = pSaGbDNpSI70912477;     pSaGbDNpSI70912477 = pSaGbDNpSI46503600;     pSaGbDNpSI46503600 = pSaGbDNpSI34548276;     pSaGbDNpSI34548276 = pSaGbDNpSI43970433;     pSaGbDNpSI43970433 = pSaGbDNpSI20419990;     pSaGbDNpSI20419990 = pSaGbDNpSI76266690;     pSaGbDNpSI76266690 = pSaGbDNpSI6072421;     pSaGbDNpSI6072421 = pSaGbDNpSI44498621;     pSaGbDNpSI44498621 = pSaGbDNpSI18081361;     pSaGbDNpSI18081361 = pSaGbDNpSI50870021;     pSaGbDNpSI50870021 = pSaGbDNpSI24881518;     pSaGbDNpSI24881518 = pSaGbDNpSI89992245;     pSaGbDNpSI89992245 = pSaGbDNpSI15478569;     pSaGbDNpSI15478569 = pSaGbDNpSI20300603;     pSaGbDNpSI20300603 = pSaGbDNpSI34161787;     pSaGbDNpSI34161787 = pSaGbDNpSI58353636;     pSaGbDNpSI58353636 = pSaGbDNpSI59340786;     pSaGbDNpSI59340786 = pSaGbDNpSI9854094;     pSaGbDNpSI9854094 = pSaGbDNpSI58309406;     pSaGbDNpSI58309406 = pSaGbDNpSI9638172;     pSaGbDNpSI9638172 = pSaGbDNpSI72499317;     pSaGbDNpSI72499317 = pSaGbDNpSI61401437;     pSaGbDNpSI61401437 = pSaGbDNpSI62017419;     pSaGbDNpSI62017419 = pSaGbDNpSI14027737;     pSaGbDNpSI14027737 = pSaGbDNpSI28517497;     pSaGbDNpSI28517497 = pSaGbDNpSI84887439;     pSaGbDNpSI84887439 = pSaGbDNpSI24887056;     pSaGbDNpSI24887056 = pSaGbDNpSI6071768;     pSaGbDNpSI6071768 = pSaGbDNpSI45562473;     pSaGbDNpSI45562473 = pSaGbDNpSI71702472;     pSaGbDNpSI71702472 = pSaGbDNpSI1266704;     pSaGbDNpSI1266704 = pSaGbDNpSI75171039;     pSaGbDNpSI75171039 = pSaGbDNpSI82069069;     pSaGbDNpSI82069069 = pSaGbDNpSI42756306;     pSaGbDNpSI42756306 = pSaGbDNpSI96518724;     pSaGbDNpSI96518724 = pSaGbDNpSI71007754;     pSaGbDNpSI71007754 = pSaGbDNpSI19624897;     pSaGbDNpSI19624897 = pSaGbDNpSI4746874;     pSaGbDNpSI4746874 = pSaGbDNpSI77521496;     pSaGbDNpSI77521496 = pSaGbDNpSI69657083;     pSaGbDNpSI69657083 = pSaGbDNpSI66458574;     pSaGbDNpSI66458574 = pSaGbDNpSI15720667;     pSaGbDNpSI15720667 = pSaGbDNpSI10781350;     pSaGbDNpSI10781350 = pSaGbDNpSI38760302;     pSaGbDNpSI38760302 = pSaGbDNpSI47262579;     pSaGbDNpSI47262579 = pSaGbDNpSI99707797;     pSaGbDNpSI99707797 = pSaGbDNpSI78026996;     pSaGbDNpSI78026996 = pSaGbDNpSI57687155;     pSaGbDNpSI57687155 = pSaGbDNpSI82633986;     pSaGbDNpSI82633986 = pSaGbDNpSI81200700;     pSaGbDNpSI81200700 = pSaGbDNpSI85488032;     pSaGbDNpSI85488032 = pSaGbDNpSI30954582;     pSaGbDNpSI30954582 = pSaGbDNpSI24957954;     pSaGbDNpSI24957954 = pSaGbDNpSI89488662;     pSaGbDNpSI89488662 = pSaGbDNpSI28247185;     pSaGbDNpSI28247185 = pSaGbDNpSI9511041;     pSaGbDNpSI9511041 = pSaGbDNpSI84486181;     pSaGbDNpSI84486181 = pSaGbDNpSI20520540;     pSaGbDNpSI20520540 = pSaGbDNpSI15452936;     pSaGbDNpSI15452936 = pSaGbDNpSI35532550;     pSaGbDNpSI35532550 = pSaGbDNpSI51379635;     pSaGbDNpSI51379635 = pSaGbDNpSI654;     pSaGbDNpSI654 = pSaGbDNpSI98936148;     pSaGbDNpSI98936148 = pSaGbDNpSI46378888;     pSaGbDNpSI46378888 = pSaGbDNpSI49603318;     pSaGbDNpSI49603318 = pSaGbDNpSI49710479;     pSaGbDNpSI49710479 = pSaGbDNpSI7923176;     pSaGbDNpSI7923176 = pSaGbDNpSI72722263;     pSaGbDNpSI72722263 = pSaGbDNpSI23781879;     pSaGbDNpSI23781879 = pSaGbDNpSI63154032;     pSaGbDNpSI63154032 = pSaGbDNpSI38728739;     pSaGbDNpSI38728739 = pSaGbDNpSI54593912;     pSaGbDNpSI54593912 = pSaGbDNpSI32332598;     pSaGbDNpSI32332598 = pSaGbDNpSI88652322;     pSaGbDNpSI88652322 = pSaGbDNpSI43179597;     pSaGbDNpSI43179597 = pSaGbDNpSI56778651;     pSaGbDNpSI56778651 = pSaGbDNpSI50620087;     pSaGbDNpSI50620087 = pSaGbDNpSI23257118;     pSaGbDNpSI23257118 = pSaGbDNpSI66765157;     pSaGbDNpSI66765157 = pSaGbDNpSI28809700;     pSaGbDNpSI28809700 = pSaGbDNpSI6860443;     pSaGbDNpSI6860443 = pSaGbDNpSI67199901;     pSaGbDNpSI67199901 = pSaGbDNpSI23437781;     pSaGbDNpSI23437781 = pSaGbDNpSI64361773;     pSaGbDNpSI64361773 = pSaGbDNpSI86214439;     pSaGbDNpSI86214439 = pSaGbDNpSI70312121;     pSaGbDNpSI70312121 = pSaGbDNpSI50213085;     pSaGbDNpSI50213085 = pSaGbDNpSI92580407;     pSaGbDNpSI92580407 = pSaGbDNpSI14509121;     pSaGbDNpSI14509121 = pSaGbDNpSI87007683;     pSaGbDNpSI87007683 = pSaGbDNpSI86521572;     pSaGbDNpSI86521572 = pSaGbDNpSI99104357;     pSaGbDNpSI99104357 = pSaGbDNpSI89293937;     pSaGbDNpSI89293937 = pSaGbDNpSI41988946;     pSaGbDNpSI41988946 = pSaGbDNpSI18277449;     pSaGbDNpSI18277449 = pSaGbDNpSI66457921;     pSaGbDNpSI66457921 = pSaGbDNpSI16784519;     pSaGbDNpSI16784519 = pSaGbDNpSI64402461;     pSaGbDNpSI64402461 = pSaGbDNpSI89156983;     pSaGbDNpSI89156983 = pSaGbDNpSI746503;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zSjkmSiKEu44822526() {     double dzdwpvCtvw19935461 = -870701728;    double dzdwpvCtvw43210335 = -731977501;    double dzdwpvCtvw71162982 = -518494083;    double dzdwpvCtvw18839724 = -632534128;    double dzdwpvCtvw70976936 = -820912223;    double dzdwpvCtvw33176947 = -99044177;    double dzdwpvCtvw55257324 = -821868085;    double dzdwpvCtvw42330013 = -475669584;    double dzdwpvCtvw13495591 = -972806094;    double dzdwpvCtvw82450147 = -741521430;    double dzdwpvCtvw11620187 = -996667040;    double dzdwpvCtvw1369760 = -409981194;    double dzdwpvCtvw47427829 = -699666578;    double dzdwpvCtvw9148357 = -389584017;    double dzdwpvCtvw37502269 = -513333868;    double dzdwpvCtvw14952569 = 88249568;    double dzdwpvCtvw58202223 = -940262776;    double dzdwpvCtvw50177714 = -545016424;    double dzdwpvCtvw30952635 = -106912268;    double dzdwpvCtvw54180506 = -869020603;    double dzdwpvCtvw10395121 = -749071670;    double dzdwpvCtvw98626647 = -495328364;    double dzdwpvCtvw64224285 = -39298620;    double dzdwpvCtvw13302951 = -528792168;    double dzdwpvCtvw30205091 = -271105963;    double dzdwpvCtvw75634349 = -380380849;    double dzdwpvCtvw96678371 = -513491657;    double dzdwpvCtvw72811156 = -728946948;    double dzdwpvCtvw11979677 = -458440215;    double dzdwpvCtvw10554342 = -157558558;    double dzdwpvCtvw38237448 = -540046832;    double dzdwpvCtvw79159152 = -490411416;    double dzdwpvCtvw16061328 = -662471967;    double dzdwpvCtvw47259373 = -117007252;    double dzdwpvCtvw93810046 = -764733299;    double dzdwpvCtvw75112612 = -705920175;    double dzdwpvCtvw8241793 = 31728845;    double dzdwpvCtvw51304412 = -520295250;    double dzdwpvCtvw60439557 = -200217647;    double dzdwpvCtvw9109252 = -561689813;    double dzdwpvCtvw80020544 = -822771625;    double dzdwpvCtvw99465766 = -318187373;    double dzdwpvCtvw77620831 = -593472790;    double dzdwpvCtvw38876309 = -17326935;    double dzdwpvCtvw82745958 = -205819388;    double dzdwpvCtvw97062820 = -532626623;    double dzdwpvCtvw24534138 = -746134042;    double dzdwpvCtvw59111575 = -921048408;    double dzdwpvCtvw95795644 = -770599461;    double dzdwpvCtvw22991483 = -189532135;    double dzdwpvCtvw88802885 = 13580915;    double dzdwpvCtvw5931968 = -694713854;    double dzdwpvCtvw69509785 = 28387663;    double dzdwpvCtvw76632135 = -89679691;    double dzdwpvCtvw34427406 = -697794161;    double dzdwpvCtvw21308813 = -275373365;    double dzdwpvCtvw78986050 = -592678882;    double dzdwpvCtvw57860031 = -989701916;    double dzdwpvCtvw88634632 = -261428166;    double dzdwpvCtvw95342587 = -340531375;    double dzdwpvCtvw36498576 = -585552520;    double dzdwpvCtvw82446168 = 7078863;    double dzdwpvCtvw30350337 = 82770631;    double dzdwpvCtvw2941250 = -715247537;    double dzdwpvCtvw44212699 = -101474598;    double dzdwpvCtvw32461034 = -406255624;    double dzdwpvCtvw85308432 = -747509228;    double dzdwpvCtvw168456 = -482659327;    double dzdwpvCtvw15338310 = -624850719;    double dzdwpvCtvw62389656 = -807413694;    double dzdwpvCtvw6710776 = -943479277;    double dzdwpvCtvw6897811 = -319967527;    double dzdwpvCtvw89738156 = -244798777;    double dzdwpvCtvw21843384 = -545222456;    double dzdwpvCtvw74159962 = 53751022;    double dzdwpvCtvw10929354 = -330884298;    double dzdwpvCtvw21005817 = -901855575;    double dzdwpvCtvw25347977 = 78028315;    double dzdwpvCtvw30556993 = -222972780;    double dzdwpvCtvw33142271 = -738479341;    double dzdwpvCtvw51100211 = -634246808;    double dzdwpvCtvw37566797 = -592443250;    double dzdwpvCtvw77015511 = -958347488;    double dzdwpvCtvw88988193 = -168908080;    double dzdwpvCtvw21751456 = -71139473;    double dzdwpvCtvw32305481 = -845332979;    double dzdwpvCtvw9649368 = -418799080;    double dzdwpvCtvw39429193 = -472792277;    double dzdwpvCtvw12831967 = -419213091;    double dzdwpvCtvw72501234 = -389359934;    double dzdwpvCtvw96126562 = -13241293;    double dzdwpvCtvw50381762 = 21430760;    double dzdwpvCtvw62669780 = -158867085;    double dzdwpvCtvw65096970 = -859686273;    double dzdwpvCtvw72610676 = -976137293;    double dzdwpvCtvw97574376 = -729850488;    double dzdwpvCtvw69115429 = -300958005;    double dzdwpvCtvw74679581 = -878225253;    double dzdwpvCtvw94663609 = -915852338;    double dzdwpvCtvw50284925 = -870701728;     dzdwpvCtvw19935461 = dzdwpvCtvw43210335;     dzdwpvCtvw43210335 = dzdwpvCtvw71162982;     dzdwpvCtvw71162982 = dzdwpvCtvw18839724;     dzdwpvCtvw18839724 = dzdwpvCtvw70976936;     dzdwpvCtvw70976936 = dzdwpvCtvw33176947;     dzdwpvCtvw33176947 = dzdwpvCtvw55257324;     dzdwpvCtvw55257324 = dzdwpvCtvw42330013;     dzdwpvCtvw42330013 = dzdwpvCtvw13495591;     dzdwpvCtvw13495591 = dzdwpvCtvw82450147;     dzdwpvCtvw82450147 = dzdwpvCtvw11620187;     dzdwpvCtvw11620187 = dzdwpvCtvw1369760;     dzdwpvCtvw1369760 = dzdwpvCtvw47427829;     dzdwpvCtvw47427829 = dzdwpvCtvw9148357;     dzdwpvCtvw9148357 = dzdwpvCtvw37502269;     dzdwpvCtvw37502269 = dzdwpvCtvw14952569;     dzdwpvCtvw14952569 = dzdwpvCtvw58202223;     dzdwpvCtvw58202223 = dzdwpvCtvw50177714;     dzdwpvCtvw50177714 = dzdwpvCtvw30952635;     dzdwpvCtvw30952635 = dzdwpvCtvw54180506;     dzdwpvCtvw54180506 = dzdwpvCtvw10395121;     dzdwpvCtvw10395121 = dzdwpvCtvw98626647;     dzdwpvCtvw98626647 = dzdwpvCtvw64224285;     dzdwpvCtvw64224285 = dzdwpvCtvw13302951;     dzdwpvCtvw13302951 = dzdwpvCtvw30205091;     dzdwpvCtvw30205091 = dzdwpvCtvw75634349;     dzdwpvCtvw75634349 = dzdwpvCtvw96678371;     dzdwpvCtvw96678371 = dzdwpvCtvw72811156;     dzdwpvCtvw72811156 = dzdwpvCtvw11979677;     dzdwpvCtvw11979677 = dzdwpvCtvw10554342;     dzdwpvCtvw10554342 = dzdwpvCtvw38237448;     dzdwpvCtvw38237448 = dzdwpvCtvw79159152;     dzdwpvCtvw79159152 = dzdwpvCtvw16061328;     dzdwpvCtvw16061328 = dzdwpvCtvw47259373;     dzdwpvCtvw47259373 = dzdwpvCtvw93810046;     dzdwpvCtvw93810046 = dzdwpvCtvw75112612;     dzdwpvCtvw75112612 = dzdwpvCtvw8241793;     dzdwpvCtvw8241793 = dzdwpvCtvw51304412;     dzdwpvCtvw51304412 = dzdwpvCtvw60439557;     dzdwpvCtvw60439557 = dzdwpvCtvw9109252;     dzdwpvCtvw9109252 = dzdwpvCtvw80020544;     dzdwpvCtvw80020544 = dzdwpvCtvw99465766;     dzdwpvCtvw99465766 = dzdwpvCtvw77620831;     dzdwpvCtvw77620831 = dzdwpvCtvw38876309;     dzdwpvCtvw38876309 = dzdwpvCtvw82745958;     dzdwpvCtvw82745958 = dzdwpvCtvw97062820;     dzdwpvCtvw97062820 = dzdwpvCtvw24534138;     dzdwpvCtvw24534138 = dzdwpvCtvw59111575;     dzdwpvCtvw59111575 = dzdwpvCtvw95795644;     dzdwpvCtvw95795644 = dzdwpvCtvw22991483;     dzdwpvCtvw22991483 = dzdwpvCtvw88802885;     dzdwpvCtvw88802885 = dzdwpvCtvw5931968;     dzdwpvCtvw5931968 = dzdwpvCtvw69509785;     dzdwpvCtvw69509785 = dzdwpvCtvw76632135;     dzdwpvCtvw76632135 = dzdwpvCtvw34427406;     dzdwpvCtvw34427406 = dzdwpvCtvw21308813;     dzdwpvCtvw21308813 = dzdwpvCtvw78986050;     dzdwpvCtvw78986050 = dzdwpvCtvw57860031;     dzdwpvCtvw57860031 = dzdwpvCtvw88634632;     dzdwpvCtvw88634632 = dzdwpvCtvw95342587;     dzdwpvCtvw95342587 = dzdwpvCtvw36498576;     dzdwpvCtvw36498576 = dzdwpvCtvw82446168;     dzdwpvCtvw82446168 = dzdwpvCtvw30350337;     dzdwpvCtvw30350337 = dzdwpvCtvw2941250;     dzdwpvCtvw2941250 = dzdwpvCtvw44212699;     dzdwpvCtvw44212699 = dzdwpvCtvw32461034;     dzdwpvCtvw32461034 = dzdwpvCtvw85308432;     dzdwpvCtvw85308432 = dzdwpvCtvw168456;     dzdwpvCtvw168456 = dzdwpvCtvw15338310;     dzdwpvCtvw15338310 = dzdwpvCtvw62389656;     dzdwpvCtvw62389656 = dzdwpvCtvw6710776;     dzdwpvCtvw6710776 = dzdwpvCtvw6897811;     dzdwpvCtvw6897811 = dzdwpvCtvw89738156;     dzdwpvCtvw89738156 = dzdwpvCtvw21843384;     dzdwpvCtvw21843384 = dzdwpvCtvw74159962;     dzdwpvCtvw74159962 = dzdwpvCtvw10929354;     dzdwpvCtvw10929354 = dzdwpvCtvw21005817;     dzdwpvCtvw21005817 = dzdwpvCtvw25347977;     dzdwpvCtvw25347977 = dzdwpvCtvw30556993;     dzdwpvCtvw30556993 = dzdwpvCtvw33142271;     dzdwpvCtvw33142271 = dzdwpvCtvw51100211;     dzdwpvCtvw51100211 = dzdwpvCtvw37566797;     dzdwpvCtvw37566797 = dzdwpvCtvw77015511;     dzdwpvCtvw77015511 = dzdwpvCtvw88988193;     dzdwpvCtvw88988193 = dzdwpvCtvw21751456;     dzdwpvCtvw21751456 = dzdwpvCtvw32305481;     dzdwpvCtvw32305481 = dzdwpvCtvw9649368;     dzdwpvCtvw9649368 = dzdwpvCtvw39429193;     dzdwpvCtvw39429193 = dzdwpvCtvw12831967;     dzdwpvCtvw12831967 = dzdwpvCtvw72501234;     dzdwpvCtvw72501234 = dzdwpvCtvw96126562;     dzdwpvCtvw96126562 = dzdwpvCtvw50381762;     dzdwpvCtvw50381762 = dzdwpvCtvw62669780;     dzdwpvCtvw62669780 = dzdwpvCtvw65096970;     dzdwpvCtvw65096970 = dzdwpvCtvw72610676;     dzdwpvCtvw72610676 = dzdwpvCtvw97574376;     dzdwpvCtvw97574376 = dzdwpvCtvw69115429;     dzdwpvCtvw69115429 = dzdwpvCtvw74679581;     dzdwpvCtvw74679581 = dzdwpvCtvw94663609;     dzdwpvCtvw94663609 = dzdwpvCtvw50284925;     dzdwpvCtvw50284925 = dzdwpvCtvw19935461;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void InEPTOIcRm60201791() {     double AJABHnjBNN71391698 = -417600175;    double AJABHnjBNN78158311 = 95392407;    double AJABHnjBNN27785732 = -241071004;    double AJABHnjBNN59559423 = -637721178;    double AJABHnjBNN74260856 = 62517823;    double AJABHnjBNN45225185 = 66177612;    double AJABHnjBNN63192922 = -363476857;    double AJABHnjBNN32128852 = 33847419;    double AJABHnjBNN73103840 = -407631459;    double AJABHnjBNN37687335 = -428502509;    double AJABHnjBNN13439788 = -208220126;    double AJABHnjBNN84719766 = -403441670;    double AJABHnjBNN1672547 = -257675837;    double AJABHnjBNN53169823 = -554872058;    double AJABHnjBNN48192731 = -159387255;    double AJABHnjBNN41254974 = -587386062;    double AJABHnjBNN8059222 = -902822630;    double AJABHnjBNN2634813 = -544046221;    double AJABHnjBNN11990146 = -844852083;    double AJABHnjBNN89169878 = -392581827;    double AJABHnjBNN44443350 = -846942349;    double AJABHnjBNN12191348 = -746805855;    double AJABHnjBNN223643 = -758030528;    double AJABHnjBNN39517065 = -932976861;    double AJABHnjBNN6594815 = -698082397;    double AJABHnjBNN75689154 = -163533316;    double AJABHnjBNN63369807 = -160109294;    double AJABHnjBNN68072806 = -295791030;    double AJABHnjBNN50892702 = -946648359;    double AJABHnjBNN60824439 = -459702831;    double AJABHnjBNN12187148 = -291588479;    double AJABHnjBNN30502022 = -180183465;    double AJABHnjBNN43568823 = -200511939;    double AJABHnjBNN47716881 = -65109940;    double AJABHnjBNN3138581 = -344917069;    double AJABHnjBNN99340172 = -751716432;    double AJABHnjBNN26740608 = -171133783;    double AJABHnjBNN42335065 = -413134918;    double AJABHnjBNN18593759 = -524873959;    double AJABHnjBNN44497687 = -986252122;    double AJABHnjBNN17586035 = -863439260;    double AJABHnjBNN80639226 = -174589647;    double AJABHnjBNN24970987 = -292902310;    double AJABHnjBNN76521548 = -119954651;    double AJABHnjBNN18732413 = -208459347;    double AJABHnjBNN94096382 = -261097795;    double AJABHnjBNN53536793 = -219132818;    double AJABHnjBNN19024788 = -462052924;    double AJABHnjBNN81786996 = -520370121;    double AJABHnjBNN11106896 = -723548535;    double AJABHnjBNN62649394 = -850052262;    double AJABHnjBNN41906795 = -362519939;    double AJABHnjBNN61478588 = 84459525;    double AJABHnjBNN25435529 = 53816798;    double AJABHnjBNN15758442 = -759316540;    double AJABHnjBNN59200350 = -670794320;    double AJABHnjBNN77934669 = -146577066;    double AJABHnjBNN88268667 = -308094143;    double AJABHnjBNN52964609 = -939638781;    double AJABHnjBNN98571701 = -773948861;    double AJABHnjBNN81855377 = -773713094;    double AJABHnjBNN95120116 = 32314172;    double AJABHnjBNN81236149 = -19504223;    double AJABHnjBNN12279401 = -947928629;    double AJABHnjBNN25500187 = -36914030;    double AJABHnjBNN82937766 = 71963338;    double AJABHnjBNN41150943 = -102929731;    double AJABHnjBNN53955665 = -92565898;    double AJABHnjBNN50031242 = -109954990;    double AJABHnjBNN48852558 = -407670824;    double AJABHnjBNN14514367 = -316252280;    double AJABHnjBNN65724157 = -389687713;    double AJABHnjBNN84041054 = 80827737;    double AJABHnjBNN67492458 = -858599962;    double AJABHnjBNN71583843 = -529142568;    double AJABHnjBNN63804124 = -572352702;    double AJABHnjBNN87220361 = -353903546;    double AJABHnjBNN23702094 = -538075877;    double AJABHnjBNN20784652 = -624517515;    double AJABHnjBNN12498433 = -336984602;    double AJABHnjBNN22152362 = -944400499;    double AJABHnjBNN44345020 = -698056371;    double AJABHnjBNN86285809 = -775420909;    double AJABHnjBNN39785806 = -123099824;    double AJABHnjBNN98175044 = -609650570;    double AJABHnjBNN70280353 = -929068541;    double AJABHnjBNN69023434 = -164642990;    double AJABHnjBNN18133294 = -154328737;    double AJABHnjBNN31958440 = -305793400;    double AJABHnjBNN43938230 = -674122750;    double AJABHnjBNN21405504 = -505139366;    double AJABHnjBNN38471941 = -863039640;    double AJABHnjBNN89370456 = -473496137;    double AJABHnjBNN20022057 = -750925098;    double AJABHnjBNN62642309 = -112539028;    double AJABHnjBNN22465919 = -795753432;    double AJABHnjBNN99403076 = -55085425;    double AJABHnjBNN12691586 = -344973681;    double AJABHnjBNN51021361 = 16959379;    double AJABHnjBNN35794647 = -417600175;     AJABHnjBNN71391698 = AJABHnjBNN78158311;     AJABHnjBNN78158311 = AJABHnjBNN27785732;     AJABHnjBNN27785732 = AJABHnjBNN59559423;     AJABHnjBNN59559423 = AJABHnjBNN74260856;     AJABHnjBNN74260856 = AJABHnjBNN45225185;     AJABHnjBNN45225185 = AJABHnjBNN63192922;     AJABHnjBNN63192922 = AJABHnjBNN32128852;     AJABHnjBNN32128852 = AJABHnjBNN73103840;     AJABHnjBNN73103840 = AJABHnjBNN37687335;     AJABHnjBNN37687335 = AJABHnjBNN13439788;     AJABHnjBNN13439788 = AJABHnjBNN84719766;     AJABHnjBNN84719766 = AJABHnjBNN1672547;     AJABHnjBNN1672547 = AJABHnjBNN53169823;     AJABHnjBNN53169823 = AJABHnjBNN48192731;     AJABHnjBNN48192731 = AJABHnjBNN41254974;     AJABHnjBNN41254974 = AJABHnjBNN8059222;     AJABHnjBNN8059222 = AJABHnjBNN2634813;     AJABHnjBNN2634813 = AJABHnjBNN11990146;     AJABHnjBNN11990146 = AJABHnjBNN89169878;     AJABHnjBNN89169878 = AJABHnjBNN44443350;     AJABHnjBNN44443350 = AJABHnjBNN12191348;     AJABHnjBNN12191348 = AJABHnjBNN223643;     AJABHnjBNN223643 = AJABHnjBNN39517065;     AJABHnjBNN39517065 = AJABHnjBNN6594815;     AJABHnjBNN6594815 = AJABHnjBNN75689154;     AJABHnjBNN75689154 = AJABHnjBNN63369807;     AJABHnjBNN63369807 = AJABHnjBNN68072806;     AJABHnjBNN68072806 = AJABHnjBNN50892702;     AJABHnjBNN50892702 = AJABHnjBNN60824439;     AJABHnjBNN60824439 = AJABHnjBNN12187148;     AJABHnjBNN12187148 = AJABHnjBNN30502022;     AJABHnjBNN30502022 = AJABHnjBNN43568823;     AJABHnjBNN43568823 = AJABHnjBNN47716881;     AJABHnjBNN47716881 = AJABHnjBNN3138581;     AJABHnjBNN3138581 = AJABHnjBNN99340172;     AJABHnjBNN99340172 = AJABHnjBNN26740608;     AJABHnjBNN26740608 = AJABHnjBNN42335065;     AJABHnjBNN42335065 = AJABHnjBNN18593759;     AJABHnjBNN18593759 = AJABHnjBNN44497687;     AJABHnjBNN44497687 = AJABHnjBNN17586035;     AJABHnjBNN17586035 = AJABHnjBNN80639226;     AJABHnjBNN80639226 = AJABHnjBNN24970987;     AJABHnjBNN24970987 = AJABHnjBNN76521548;     AJABHnjBNN76521548 = AJABHnjBNN18732413;     AJABHnjBNN18732413 = AJABHnjBNN94096382;     AJABHnjBNN94096382 = AJABHnjBNN53536793;     AJABHnjBNN53536793 = AJABHnjBNN19024788;     AJABHnjBNN19024788 = AJABHnjBNN81786996;     AJABHnjBNN81786996 = AJABHnjBNN11106896;     AJABHnjBNN11106896 = AJABHnjBNN62649394;     AJABHnjBNN62649394 = AJABHnjBNN41906795;     AJABHnjBNN41906795 = AJABHnjBNN61478588;     AJABHnjBNN61478588 = AJABHnjBNN25435529;     AJABHnjBNN25435529 = AJABHnjBNN15758442;     AJABHnjBNN15758442 = AJABHnjBNN59200350;     AJABHnjBNN59200350 = AJABHnjBNN77934669;     AJABHnjBNN77934669 = AJABHnjBNN88268667;     AJABHnjBNN88268667 = AJABHnjBNN52964609;     AJABHnjBNN52964609 = AJABHnjBNN98571701;     AJABHnjBNN98571701 = AJABHnjBNN81855377;     AJABHnjBNN81855377 = AJABHnjBNN95120116;     AJABHnjBNN95120116 = AJABHnjBNN81236149;     AJABHnjBNN81236149 = AJABHnjBNN12279401;     AJABHnjBNN12279401 = AJABHnjBNN25500187;     AJABHnjBNN25500187 = AJABHnjBNN82937766;     AJABHnjBNN82937766 = AJABHnjBNN41150943;     AJABHnjBNN41150943 = AJABHnjBNN53955665;     AJABHnjBNN53955665 = AJABHnjBNN50031242;     AJABHnjBNN50031242 = AJABHnjBNN48852558;     AJABHnjBNN48852558 = AJABHnjBNN14514367;     AJABHnjBNN14514367 = AJABHnjBNN65724157;     AJABHnjBNN65724157 = AJABHnjBNN84041054;     AJABHnjBNN84041054 = AJABHnjBNN67492458;     AJABHnjBNN67492458 = AJABHnjBNN71583843;     AJABHnjBNN71583843 = AJABHnjBNN63804124;     AJABHnjBNN63804124 = AJABHnjBNN87220361;     AJABHnjBNN87220361 = AJABHnjBNN23702094;     AJABHnjBNN23702094 = AJABHnjBNN20784652;     AJABHnjBNN20784652 = AJABHnjBNN12498433;     AJABHnjBNN12498433 = AJABHnjBNN22152362;     AJABHnjBNN22152362 = AJABHnjBNN44345020;     AJABHnjBNN44345020 = AJABHnjBNN86285809;     AJABHnjBNN86285809 = AJABHnjBNN39785806;     AJABHnjBNN39785806 = AJABHnjBNN98175044;     AJABHnjBNN98175044 = AJABHnjBNN70280353;     AJABHnjBNN70280353 = AJABHnjBNN69023434;     AJABHnjBNN69023434 = AJABHnjBNN18133294;     AJABHnjBNN18133294 = AJABHnjBNN31958440;     AJABHnjBNN31958440 = AJABHnjBNN43938230;     AJABHnjBNN43938230 = AJABHnjBNN21405504;     AJABHnjBNN21405504 = AJABHnjBNN38471941;     AJABHnjBNN38471941 = AJABHnjBNN89370456;     AJABHnjBNN89370456 = AJABHnjBNN20022057;     AJABHnjBNN20022057 = AJABHnjBNN62642309;     AJABHnjBNN62642309 = AJABHnjBNN22465919;     AJABHnjBNN22465919 = AJABHnjBNN99403076;     AJABHnjBNN99403076 = AJABHnjBNN12691586;     AJABHnjBNN12691586 = AJABHnjBNN51021361;     AJABHnjBNN51021361 = AJABHnjBNN35794647;     AJABHnjBNN35794647 = AJABHnjBNN71391698;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XfTvnrCrRW44934432() {     double eNLKWDDEBZ9776086 = -232625684;    double eNLKWDDEBZ84181710 = -245229133;    double eNLKWDDEBZ75791283 = -537634695;    double eNLKWDDEBZ43062457 = -828164687;    double eNLKWDDEBZ19433762 = -639510286;    double eNLKWDDEBZ35933667 = -466861081;    double eNLKWDDEBZ52626853 = -982347200;    double eNLKWDDEBZ35429987 = -282998083;    double eNLKWDDEBZ28357615 = -338321456;    double eNLKWDDEBZ69785395 = -193537923;    double eNLKWDDEBZ14619513 = -714389884;    double eNLKWDDEBZ23404573 = -783526667;    double eNLKWDDEBZ84690799 = -982389911;    double eNLKWDDEBZ86665177 = -171505447;    double eNLKWDDEBZ37423954 = -29493340;    double eNLKWDDEBZ28271991 = -908877627;    double eNLKWDDEBZ51903637 = -615687752;    double eNLKWDDEBZ61308136 = -921390641;    double eNLKWDDEBZ33758957 = -301330603;    double eNLKWDDEBZ8838316 = -842216439;    double eNLKWDDEBZ66287823 = -179025647;    double eNLKWDDEBZ86149742 = -316413366;    double eNLKWDDEBZ68990942 = -915210844;    double eNLKWDDEBZ98438019 = -290329808;    double eNLKWDDEBZ10506834 = -722632757;    double eNLKWDDEBZ53348782 = -702003200;    double eNLKWDDEBZ16730089 = -858029283;    double eNLKWDDEBZ28517482 = -581317390;    double eNLKWDDEBZ50030073 = -105018820;    double eNLKWDDEBZ14718306 = -647635800;    double eNLKWDDEBZ11844421 = -672465791;    double eNLKWDDEBZ1563063 = -749055454;    double eNLKWDDEBZ39896978 = -120299428;    double eNLKWDDEBZ77814651 = -272645665;    double eNLKWDDEBZ22888025 = -37757012;    double eNLKWDDEBZ13699759 = -898342759;    double eNLKWDDEBZ89267317 = -242248321;    double eNLKWDDEBZ35776416 = -190944476;    double eNLKWDDEBZ94665530 = -499700026;    double eNLKWDDEBZ35800305 = -981775915;    double eNLKWDDEBZ50457289 = -538085646;    double eNLKWDDEBZ60955950 = -194355321;    double eNLKWDDEBZ88487955 = -326681628;    double eNLKWDDEBZ73479568 = -715613606;    double eNLKWDDEBZ22685675 = -193731621;    double eNLKWDDEBZ53268897 = -132874431;    double eNLKWDDEBZ75491681 = -886555486;    double eNLKWDDEBZ5810364 = -693065838;    double eNLKWDDEBZ39345587 = -556120600;    double eNLKWDDEBZ71776465 = -908558379;    double eNLKWDDEBZ64594396 = 5514842;    double eNLKWDDEBZ68247406 = -247492960;    double eNLKWDDEBZ63140645 = -875334290;    double eNLKWDDEBZ18533622 = -986859023;    double eNLKWDDEBZ78646608 = -615052888;    double eNLKWDDEBZ23626343 = -916212318;    double eNLKWDDEBZ15190768 = -330018290;    double eNLKWDDEBZ77353263 = -147304887;    double eNLKWDDEBZ32555624 = -5531931;    double eNLKWDDEBZ66084979 = -937507086;    double eNLKWDDEBZ19203578 = -608831798;    double eNLKWDDEBZ24109372 = -301029810;    double eNLKWDDEBZ85399913 = -77979263;    double eNLKWDDEBZ13639310 = -690685657;    double eNLKWDDEBZ57940974 = -521072133;    double eNLKWDDEBZ13056450 = -965334430;    double eNLKWDDEBZ83507595 = -563227239;    double eNLKWDDEBZ6876148 = -609744247;    double eNLKWDDEBZ63777153 = -33748436;    double eNLKWDDEBZ23724195 = -131150581;    double eNLKWDDEBZ39004673 = -566629306;    double eNLKWDDEBZ16127222 = -324743276;    double eNLKWDDEBZ66642606 = -321690615;    double eNLKWDDEBZ97958652 = -319554689;    double eNLKWDDEBZ58381027 = -204130794;    double eNLKWDDEBZ5331874 = -984670326;    double eNLKWDDEBZ97661786 = -989731739;    double eNLKWDDEBZ95511373 = -99597238;    double eNLKWDDEBZ75752345 = 3401812;    double eNLKWDDEBZ57237937 = -489758327;    double eNLKWDDEBZ77857100 = -815447715;    double eNLKWDDEBZ10919726 = -64963446;    double eNLKWDDEBZ89171894 = 74803209;    double eNLKWDDEBZ78253608 = -196460442;    double eNLKWDDEBZ50123910 = -553150642;    double eNLKWDDEBZ43597014 = -324972832;    double eNLKWDDEBZ38422418 = -873721165;    double eNLKWDDEBZ21363356 = -133440406;    double eNLKWDDEBZ99168042 = -657592777;    double eNLKWDDEBZ99261681 = -121544694;    double eNLKWDDEBZ98508991 = -468324470;    double eNLKWDDEBZ11914055 = 5056565;    double eNLKWDDEBZ3220793 = -85412546;    double eNLKWDDEBZ28580551 = -562192940;    double eNLKWDDEBZ16596727 = -272944117;    double eNLKWDDEBZ26347918 = -137055836;    double eNLKWDDEBZ75556036 = -16376058;    double eNLKWDDEBZ74848646 = -635995971;    double eNLKWDDEBZ15538595 = -94541474;    double eNLKWDDEBZ9629225 = -232625684;     eNLKWDDEBZ9776086 = eNLKWDDEBZ84181710;     eNLKWDDEBZ84181710 = eNLKWDDEBZ75791283;     eNLKWDDEBZ75791283 = eNLKWDDEBZ43062457;     eNLKWDDEBZ43062457 = eNLKWDDEBZ19433762;     eNLKWDDEBZ19433762 = eNLKWDDEBZ35933667;     eNLKWDDEBZ35933667 = eNLKWDDEBZ52626853;     eNLKWDDEBZ52626853 = eNLKWDDEBZ35429987;     eNLKWDDEBZ35429987 = eNLKWDDEBZ28357615;     eNLKWDDEBZ28357615 = eNLKWDDEBZ69785395;     eNLKWDDEBZ69785395 = eNLKWDDEBZ14619513;     eNLKWDDEBZ14619513 = eNLKWDDEBZ23404573;     eNLKWDDEBZ23404573 = eNLKWDDEBZ84690799;     eNLKWDDEBZ84690799 = eNLKWDDEBZ86665177;     eNLKWDDEBZ86665177 = eNLKWDDEBZ37423954;     eNLKWDDEBZ37423954 = eNLKWDDEBZ28271991;     eNLKWDDEBZ28271991 = eNLKWDDEBZ51903637;     eNLKWDDEBZ51903637 = eNLKWDDEBZ61308136;     eNLKWDDEBZ61308136 = eNLKWDDEBZ33758957;     eNLKWDDEBZ33758957 = eNLKWDDEBZ8838316;     eNLKWDDEBZ8838316 = eNLKWDDEBZ66287823;     eNLKWDDEBZ66287823 = eNLKWDDEBZ86149742;     eNLKWDDEBZ86149742 = eNLKWDDEBZ68990942;     eNLKWDDEBZ68990942 = eNLKWDDEBZ98438019;     eNLKWDDEBZ98438019 = eNLKWDDEBZ10506834;     eNLKWDDEBZ10506834 = eNLKWDDEBZ53348782;     eNLKWDDEBZ53348782 = eNLKWDDEBZ16730089;     eNLKWDDEBZ16730089 = eNLKWDDEBZ28517482;     eNLKWDDEBZ28517482 = eNLKWDDEBZ50030073;     eNLKWDDEBZ50030073 = eNLKWDDEBZ14718306;     eNLKWDDEBZ14718306 = eNLKWDDEBZ11844421;     eNLKWDDEBZ11844421 = eNLKWDDEBZ1563063;     eNLKWDDEBZ1563063 = eNLKWDDEBZ39896978;     eNLKWDDEBZ39896978 = eNLKWDDEBZ77814651;     eNLKWDDEBZ77814651 = eNLKWDDEBZ22888025;     eNLKWDDEBZ22888025 = eNLKWDDEBZ13699759;     eNLKWDDEBZ13699759 = eNLKWDDEBZ89267317;     eNLKWDDEBZ89267317 = eNLKWDDEBZ35776416;     eNLKWDDEBZ35776416 = eNLKWDDEBZ94665530;     eNLKWDDEBZ94665530 = eNLKWDDEBZ35800305;     eNLKWDDEBZ35800305 = eNLKWDDEBZ50457289;     eNLKWDDEBZ50457289 = eNLKWDDEBZ60955950;     eNLKWDDEBZ60955950 = eNLKWDDEBZ88487955;     eNLKWDDEBZ88487955 = eNLKWDDEBZ73479568;     eNLKWDDEBZ73479568 = eNLKWDDEBZ22685675;     eNLKWDDEBZ22685675 = eNLKWDDEBZ53268897;     eNLKWDDEBZ53268897 = eNLKWDDEBZ75491681;     eNLKWDDEBZ75491681 = eNLKWDDEBZ5810364;     eNLKWDDEBZ5810364 = eNLKWDDEBZ39345587;     eNLKWDDEBZ39345587 = eNLKWDDEBZ71776465;     eNLKWDDEBZ71776465 = eNLKWDDEBZ64594396;     eNLKWDDEBZ64594396 = eNLKWDDEBZ68247406;     eNLKWDDEBZ68247406 = eNLKWDDEBZ63140645;     eNLKWDDEBZ63140645 = eNLKWDDEBZ18533622;     eNLKWDDEBZ18533622 = eNLKWDDEBZ78646608;     eNLKWDDEBZ78646608 = eNLKWDDEBZ23626343;     eNLKWDDEBZ23626343 = eNLKWDDEBZ15190768;     eNLKWDDEBZ15190768 = eNLKWDDEBZ77353263;     eNLKWDDEBZ77353263 = eNLKWDDEBZ32555624;     eNLKWDDEBZ32555624 = eNLKWDDEBZ66084979;     eNLKWDDEBZ66084979 = eNLKWDDEBZ19203578;     eNLKWDDEBZ19203578 = eNLKWDDEBZ24109372;     eNLKWDDEBZ24109372 = eNLKWDDEBZ85399913;     eNLKWDDEBZ85399913 = eNLKWDDEBZ13639310;     eNLKWDDEBZ13639310 = eNLKWDDEBZ57940974;     eNLKWDDEBZ57940974 = eNLKWDDEBZ13056450;     eNLKWDDEBZ13056450 = eNLKWDDEBZ83507595;     eNLKWDDEBZ83507595 = eNLKWDDEBZ6876148;     eNLKWDDEBZ6876148 = eNLKWDDEBZ63777153;     eNLKWDDEBZ63777153 = eNLKWDDEBZ23724195;     eNLKWDDEBZ23724195 = eNLKWDDEBZ39004673;     eNLKWDDEBZ39004673 = eNLKWDDEBZ16127222;     eNLKWDDEBZ16127222 = eNLKWDDEBZ66642606;     eNLKWDDEBZ66642606 = eNLKWDDEBZ97958652;     eNLKWDDEBZ97958652 = eNLKWDDEBZ58381027;     eNLKWDDEBZ58381027 = eNLKWDDEBZ5331874;     eNLKWDDEBZ5331874 = eNLKWDDEBZ97661786;     eNLKWDDEBZ97661786 = eNLKWDDEBZ95511373;     eNLKWDDEBZ95511373 = eNLKWDDEBZ75752345;     eNLKWDDEBZ75752345 = eNLKWDDEBZ57237937;     eNLKWDDEBZ57237937 = eNLKWDDEBZ77857100;     eNLKWDDEBZ77857100 = eNLKWDDEBZ10919726;     eNLKWDDEBZ10919726 = eNLKWDDEBZ89171894;     eNLKWDDEBZ89171894 = eNLKWDDEBZ78253608;     eNLKWDDEBZ78253608 = eNLKWDDEBZ50123910;     eNLKWDDEBZ50123910 = eNLKWDDEBZ43597014;     eNLKWDDEBZ43597014 = eNLKWDDEBZ38422418;     eNLKWDDEBZ38422418 = eNLKWDDEBZ21363356;     eNLKWDDEBZ21363356 = eNLKWDDEBZ99168042;     eNLKWDDEBZ99168042 = eNLKWDDEBZ99261681;     eNLKWDDEBZ99261681 = eNLKWDDEBZ98508991;     eNLKWDDEBZ98508991 = eNLKWDDEBZ11914055;     eNLKWDDEBZ11914055 = eNLKWDDEBZ3220793;     eNLKWDDEBZ3220793 = eNLKWDDEBZ28580551;     eNLKWDDEBZ28580551 = eNLKWDDEBZ16596727;     eNLKWDDEBZ16596727 = eNLKWDDEBZ26347918;     eNLKWDDEBZ26347918 = eNLKWDDEBZ75556036;     eNLKWDDEBZ75556036 = eNLKWDDEBZ74848646;     eNLKWDDEBZ74848646 = eNLKWDDEBZ15538595;     eNLKWDDEBZ15538595 = eNLKWDDEBZ9629225;     eNLKWDDEBZ9629225 = eNLKWDDEBZ9776086;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TZWWvlegQq81909603() {     double CHVlgDcHbg77502333 = -402026203;    double CHVlgDcHbg60103908 = -756802493;    double CHVlgDcHbg45141127 = -349032539;    double CHVlgDcHbg71714065 = -649010641;    double CHVlgDcHbg51996447 = -926487361;    double CHVlgDcHbg71447819 = -997751434;    double CHVlgDcHbg92229224 = -206978301;    double CHVlgDcHbg51102793 = 42796204;    double CHVlgDcHbg20486500 = -989310194;    double CHVlgDcHbg75556507 = -394284858;    double CHVlgDcHbg93870683 = -45129784;    double CHVlgDcHbg30834486 = -583326225;    double CHVlgDcHbg19734579 = -719225400;    double CHVlgDcHbg90157719 = -332263677;    double CHVlgDcHbg89107266 = -812562255;    double CHVlgDcHbg57324915 = -893181256;    double CHVlgDcHbg57747985 = -238982313;    double CHVlgDcHbg22688499 = -606640482;    double CHVlgDcHbg94248256 = -768603448;    double CHVlgDcHbg41793806 = -261509198;    double CHVlgDcHbg12665968 = -412896173;    double CHVlgDcHbg77008637 = -64727455;    double CHVlgDcHbg72692830 = -963505857;    double CHVlgDcHbg67159547 = 63797628;    double CHVlgDcHbg78737154 = -397972271;    double CHVlgDcHbg16984906 = 49605428;    double CHVlgDcHbg32051167 = -361571210;    double CHVlgDcHbg16583457 = -970686966;    double CHVlgDcHbg29703404 = -715101394;    double CHVlgDcHbg5529946 = 47394928;    double CHVlgDcHbg37842376 = -592002652;    double CHVlgDcHbg12836501 = -281452045;    double CHVlgDcHbg38732195 = -295069528;    double CHVlgDcHbg42830283 = -793333438;    double CHVlgDcHbg46971270 = -142964102;    double CHVlgDcHbg10894276 = 54491696;    double CHVlgDcHbg19943910 = -224423031;    double CHVlgDcHbg46342957 = -179903603;    double CHVlgDcHbg27517609 = -131478884;    double CHVlgDcHbg3872518 = 30876969;    double CHVlgDcHbg11110927 = 83342948;    double CHVlgDcHbg98487343 = -638524028;    double CHVlgDcHbg63321325 = -932837150;    double CHVlgDcHbg23161187 = -796262033;    double CHVlgDcHbg14702930 = -343616903;    double CHVlgDcHbg46463546 = -58358601;    double CHVlgDcHbg99013160 = -819188977;    double CHVlgDcHbg2365310 = 19290186;    double CHVlgDcHbg10121115 = -169870995;    double CHVlgDcHbg44063972 = -397584225;    double CHVlgDcHbg11609443 = -270900940;    double CHVlgDcHbg67263771 = -545392012;    double CHVlgDcHbg85175393 = -699384068;    double CHVlgDcHbg31654680 = -798573199;    double CHVlgDcHbg69243636 = -116747605;    double CHVlgDcHbg493697 = -237298748;    double CHVlgDcHbg87411077 = -793296637;    double CHVlgDcHbg77981580 = -312830168;    double CHVlgDcHbg92976910 = -151038371;    double CHVlgDcHbg35011541 = -876092790;    double CHVlgDcHbg39396652 = -536180225;    double CHVlgDcHbg75645767 = -236291336;    double CHVlgDcHbg21399389 = -242102403;    double CHVlgDcHbg14956554 = -936705123;    double CHVlgDcHbg37714132 = -802282206;    double CHVlgDcHbg81034182 = -763677739;    double CHVlgDcHbg92102291 = -188256697;    double CHVlgDcHbg76904296 = -925891963;    double CHVlgDcHbg43186449 = -89299575;    double CHVlgDcHbg78212991 = -767053952;    double CHVlgDcHbg37381005 = -568758226;    double CHVlgDcHbg11405028 = 40921290;    double CHVlgDcHbg95170889 = -375161598;    double CHVlgDcHbg90375738 = -699480418;    double CHVlgDcHbg30682880 = -244852146;    double CHVlgDcHbg14178624 = -774372145;    double CHVlgDcHbg13687313 = -131890306;    double CHVlgDcHbg49531643 = -67243824;    double CHVlgDcHbg52456617 = -592585469;    double CHVlgDcHbg32273609 = -239613671;    double CHVlgDcHbg17971746 = -131205595;    double CHVlgDcHbg29685858 = -280861397;    double CHVlgDcHbg6462342 = -700815971;    double CHVlgDcHbg85639432 = -217517170;    double CHVlgDcHbg93920503 = -681704132;    double CHVlgDcHbg70578604 = 53389359;    double CHVlgDcHbg27661107 = -582067978;    double CHVlgDcHbg7077515 = -496496330;    double CHVlgDcHbg73586646 = -576585833;    double CHVlgDcHbg46477574 = -905665355;    double CHVlgDcHbg23483198 = -152211668;    double CHVlgDcHbg41962329 = -911592864;    double CHVlgDcHbg53366046 = 71134767;    double CHVlgDcHbg92506068 = -255386095;    double CHVlgDcHbg64475865 = -432942806;    double CHVlgDcHbg35465159 = -680365717;    double CHVlgDcHbg77087955 = -296421626;    double CHVlgDcHbg48364771 = -996132028;    double CHVlgDcHbg85447055 = -993979827;    double CHVlgDcHbg33668748 = -402026203;     CHVlgDcHbg77502333 = CHVlgDcHbg60103908;     CHVlgDcHbg60103908 = CHVlgDcHbg45141127;     CHVlgDcHbg45141127 = CHVlgDcHbg71714065;     CHVlgDcHbg71714065 = CHVlgDcHbg51996447;     CHVlgDcHbg51996447 = CHVlgDcHbg71447819;     CHVlgDcHbg71447819 = CHVlgDcHbg92229224;     CHVlgDcHbg92229224 = CHVlgDcHbg51102793;     CHVlgDcHbg51102793 = CHVlgDcHbg20486500;     CHVlgDcHbg20486500 = CHVlgDcHbg75556507;     CHVlgDcHbg75556507 = CHVlgDcHbg93870683;     CHVlgDcHbg93870683 = CHVlgDcHbg30834486;     CHVlgDcHbg30834486 = CHVlgDcHbg19734579;     CHVlgDcHbg19734579 = CHVlgDcHbg90157719;     CHVlgDcHbg90157719 = CHVlgDcHbg89107266;     CHVlgDcHbg89107266 = CHVlgDcHbg57324915;     CHVlgDcHbg57324915 = CHVlgDcHbg57747985;     CHVlgDcHbg57747985 = CHVlgDcHbg22688499;     CHVlgDcHbg22688499 = CHVlgDcHbg94248256;     CHVlgDcHbg94248256 = CHVlgDcHbg41793806;     CHVlgDcHbg41793806 = CHVlgDcHbg12665968;     CHVlgDcHbg12665968 = CHVlgDcHbg77008637;     CHVlgDcHbg77008637 = CHVlgDcHbg72692830;     CHVlgDcHbg72692830 = CHVlgDcHbg67159547;     CHVlgDcHbg67159547 = CHVlgDcHbg78737154;     CHVlgDcHbg78737154 = CHVlgDcHbg16984906;     CHVlgDcHbg16984906 = CHVlgDcHbg32051167;     CHVlgDcHbg32051167 = CHVlgDcHbg16583457;     CHVlgDcHbg16583457 = CHVlgDcHbg29703404;     CHVlgDcHbg29703404 = CHVlgDcHbg5529946;     CHVlgDcHbg5529946 = CHVlgDcHbg37842376;     CHVlgDcHbg37842376 = CHVlgDcHbg12836501;     CHVlgDcHbg12836501 = CHVlgDcHbg38732195;     CHVlgDcHbg38732195 = CHVlgDcHbg42830283;     CHVlgDcHbg42830283 = CHVlgDcHbg46971270;     CHVlgDcHbg46971270 = CHVlgDcHbg10894276;     CHVlgDcHbg10894276 = CHVlgDcHbg19943910;     CHVlgDcHbg19943910 = CHVlgDcHbg46342957;     CHVlgDcHbg46342957 = CHVlgDcHbg27517609;     CHVlgDcHbg27517609 = CHVlgDcHbg3872518;     CHVlgDcHbg3872518 = CHVlgDcHbg11110927;     CHVlgDcHbg11110927 = CHVlgDcHbg98487343;     CHVlgDcHbg98487343 = CHVlgDcHbg63321325;     CHVlgDcHbg63321325 = CHVlgDcHbg23161187;     CHVlgDcHbg23161187 = CHVlgDcHbg14702930;     CHVlgDcHbg14702930 = CHVlgDcHbg46463546;     CHVlgDcHbg46463546 = CHVlgDcHbg99013160;     CHVlgDcHbg99013160 = CHVlgDcHbg2365310;     CHVlgDcHbg2365310 = CHVlgDcHbg10121115;     CHVlgDcHbg10121115 = CHVlgDcHbg44063972;     CHVlgDcHbg44063972 = CHVlgDcHbg11609443;     CHVlgDcHbg11609443 = CHVlgDcHbg67263771;     CHVlgDcHbg67263771 = CHVlgDcHbg85175393;     CHVlgDcHbg85175393 = CHVlgDcHbg31654680;     CHVlgDcHbg31654680 = CHVlgDcHbg69243636;     CHVlgDcHbg69243636 = CHVlgDcHbg493697;     CHVlgDcHbg493697 = CHVlgDcHbg87411077;     CHVlgDcHbg87411077 = CHVlgDcHbg77981580;     CHVlgDcHbg77981580 = CHVlgDcHbg92976910;     CHVlgDcHbg92976910 = CHVlgDcHbg35011541;     CHVlgDcHbg35011541 = CHVlgDcHbg39396652;     CHVlgDcHbg39396652 = CHVlgDcHbg75645767;     CHVlgDcHbg75645767 = CHVlgDcHbg21399389;     CHVlgDcHbg21399389 = CHVlgDcHbg14956554;     CHVlgDcHbg14956554 = CHVlgDcHbg37714132;     CHVlgDcHbg37714132 = CHVlgDcHbg81034182;     CHVlgDcHbg81034182 = CHVlgDcHbg92102291;     CHVlgDcHbg92102291 = CHVlgDcHbg76904296;     CHVlgDcHbg76904296 = CHVlgDcHbg43186449;     CHVlgDcHbg43186449 = CHVlgDcHbg78212991;     CHVlgDcHbg78212991 = CHVlgDcHbg37381005;     CHVlgDcHbg37381005 = CHVlgDcHbg11405028;     CHVlgDcHbg11405028 = CHVlgDcHbg95170889;     CHVlgDcHbg95170889 = CHVlgDcHbg90375738;     CHVlgDcHbg90375738 = CHVlgDcHbg30682880;     CHVlgDcHbg30682880 = CHVlgDcHbg14178624;     CHVlgDcHbg14178624 = CHVlgDcHbg13687313;     CHVlgDcHbg13687313 = CHVlgDcHbg49531643;     CHVlgDcHbg49531643 = CHVlgDcHbg52456617;     CHVlgDcHbg52456617 = CHVlgDcHbg32273609;     CHVlgDcHbg32273609 = CHVlgDcHbg17971746;     CHVlgDcHbg17971746 = CHVlgDcHbg29685858;     CHVlgDcHbg29685858 = CHVlgDcHbg6462342;     CHVlgDcHbg6462342 = CHVlgDcHbg85639432;     CHVlgDcHbg85639432 = CHVlgDcHbg93920503;     CHVlgDcHbg93920503 = CHVlgDcHbg70578604;     CHVlgDcHbg70578604 = CHVlgDcHbg27661107;     CHVlgDcHbg27661107 = CHVlgDcHbg7077515;     CHVlgDcHbg7077515 = CHVlgDcHbg73586646;     CHVlgDcHbg73586646 = CHVlgDcHbg46477574;     CHVlgDcHbg46477574 = CHVlgDcHbg23483198;     CHVlgDcHbg23483198 = CHVlgDcHbg41962329;     CHVlgDcHbg41962329 = CHVlgDcHbg53366046;     CHVlgDcHbg53366046 = CHVlgDcHbg92506068;     CHVlgDcHbg92506068 = CHVlgDcHbg64475865;     CHVlgDcHbg64475865 = CHVlgDcHbg35465159;     CHVlgDcHbg35465159 = CHVlgDcHbg77087955;     CHVlgDcHbg77087955 = CHVlgDcHbg48364771;     CHVlgDcHbg48364771 = CHVlgDcHbg85447055;     CHVlgDcHbg85447055 = CHVlgDcHbg33668748;     CHVlgDcHbg33668748 = CHVlgDcHbg77502333;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ieqbHKQUGI57921723() {     double ssBthEQDFK64424011 = -542677408;    double ssBthEQDFK69751646 = -867389066;    double ssBthEQDFK37837141 = -880147922;    double ssBthEQDFK99577259 = -654807932;    double ssBthEQDFK2725534 = -780300839;    double ssBthEQDFK84913496 = -295444729;    double ssBthEQDFK42274893 = -988776340;    double ssBthEQDFK33819141 = -487743617;    double ssBthEQDFK98872189 = -98820896;    double ssBthEQDFK49056894 = -109146064;    double ssBthEQDFK13551413 = -199218527;    double ssBthEQDFK460964 = -705429109;    double ssBthEQDFK80361027 = -807588690;    double ssBthEQDFK33475829 = -128762076;    double ssBthEQDFK12820137 = -999327805;    double ssBthEQDFK92604073 = -871832842;    double ssBthEQDFK7588160 = -908902150;    double ssBthEQDFK51905256 = -282026726;    double ssBthEQDFK55407827 = -105124418;    double ssBthEQDFK98546634 = -699607037;    double ssBthEQDFK80131635 = -457575166;    double ssBthEQDFK15698597 = -992849358;    double ssBthEQDFK42339170 = -860912107;    double ssBthEQDFK43516497 = -970291147;    double ssBthEQDFK34702140 = -422240050;    double ssBthEQDFK11163806 = -613917917;    double ssBthEQDFK88941594 = -613673275;    double ssBthEQDFK17170007 = -98336233;    double ssBthEQDFK2606198 = -31334026;    double ssBthEQDFK85243584 = -613825142;    double ssBthEQDFK96962628 = -508431551;    double ssBthEQDFK17278531 = -452373747;    double ssBthEQDFK93005277 = -878761262;    double ssBthEQDFK72753381 = -929448207;    double ssBthEQDFK39750219 = -514934199;    double ssBthEQDFK43854490 = -126104121;    double ssBthEQDFK75913173 = -192328320;    double ssBthEQDFK18671335 = -60136173;    double ssBthEQDFK80748775 = -494330056;    double ssBthEQDFK31659593 = -249516200;    double ssBthEQDFK94272357 = -738579702;    double ssBthEQDFK83328269 = -995679510;    double ssBthEQDFK39771499 = -726317202;    double ssBthEQDFK41705866 = -846257715;    double ssBthEQDFK66687790 = -799508621;    double ssBthEQDFK49030467 = -13708735;    double ssBthEQDFK19663187 = -294893492;    double ssBthEQDFK4621254 = -179479567;    double ssBthEQDFK346744 = -19614675;    double ssBthEQDFK36663551 = -735602554;    double ssBthEQDFK52967305 = -330255667;    double ssBthEQDFK72176813 = -44704695;    double ssBthEQDFK70316996 = -507303753;    double ssBthEQDFK86199649 = -314665359;    double ssBthEQDFK77790088 = -767860852;    double ssBthEQDFK48725414 = -549828051;    double ssBthEQDFK27412476 = 93523040;    double ssBthEQDFK94320644 = -909856775;    double ssBthEQDFK64875120 = -132567882;    double ssBthEQDFK91561728 = -66382922;    double ssBthEQDFK95971901 = -681771454;    double ssBthEQDFK25104886 = -790440107;    double ssBthEQDFK31212944 = -356409592;    double ssBthEQDFK13628605 = -484995755;    double ssBthEQDFK52094266 = -600714513;    double ssBthEQDFK96272882 = -746844781;    double ssBthEQDFK7455686 = -826667848;    double ssBthEQDFK7607647 = -878140483;    double ssBthEQDFK93725609 = -613827877;    double ssBthEQDFK68965646 = -773223685;    double ssBthEQDFK16690901 = -579504522;    double ssBthEQDFK88916825 = -748765977;    double ssBthEQDFK71156481 = -787696670;    double ssBthEQDFK23748235 = -855608219;    double ssBthEQDFK4274277 = -961027335;    double ssBthEQDFK96803366 = -461895656;    double ssBthEQDFK75927098 = -166532156;    double ssBthEQDFK633305 = 85345608;    double ssBthEQDFK76828707 = -70782527;    double ssBthEQDFK85671672 = -308531316;    double ssBthEQDFK91500619 = -219024426;    double ssBthEQDFK84320341 = -334193708;    double ssBthEQDFK16823264 = 21278441;    double ssBthEQDFK65942646 = -295731472;    double ssBthEQDFK32276279 = -183569475;    double ssBthEQDFK24785815 = -363726857;    double ssBthEQDFK46961534 = -945069995;    double ssBthEQDFK6805628 = -464095903;    double ssBthEQDFK94963292 = -61587355;    double ssBthEQDFK91024804 = -965106149;    double ssBthEQDFK16442015 = -119627161;    double ssBthEQDFK81592529 = -282471546;    double ssBthEQDFK53796214 = -927568292;    double ssBthEQDFK89187047 = -327947134;    double ssBthEQDFK35687692 = -567744746;    double ssBthEQDFK69167471 = -948139596;    double ssBthEQDFK52115325 = -539269919;    double ssBthEQDFK26142894 = -141321448;    double ssBthEQDFK89611600 = -145543203;    double ssBthEQDFK70414907 = -542677408;     ssBthEQDFK64424011 = ssBthEQDFK69751646;     ssBthEQDFK69751646 = ssBthEQDFK37837141;     ssBthEQDFK37837141 = ssBthEQDFK99577259;     ssBthEQDFK99577259 = ssBthEQDFK2725534;     ssBthEQDFK2725534 = ssBthEQDFK84913496;     ssBthEQDFK84913496 = ssBthEQDFK42274893;     ssBthEQDFK42274893 = ssBthEQDFK33819141;     ssBthEQDFK33819141 = ssBthEQDFK98872189;     ssBthEQDFK98872189 = ssBthEQDFK49056894;     ssBthEQDFK49056894 = ssBthEQDFK13551413;     ssBthEQDFK13551413 = ssBthEQDFK460964;     ssBthEQDFK460964 = ssBthEQDFK80361027;     ssBthEQDFK80361027 = ssBthEQDFK33475829;     ssBthEQDFK33475829 = ssBthEQDFK12820137;     ssBthEQDFK12820137 = ssBthEQDFK92604073;     ssBthEQDFK92604073 = ssBthEQDFK7588160;     ssBthEQDFK7588160 = ssBthEQDFK51905256;     ssBthEQDFK51905256 = ssBthEQDFK55407827;     ssBthEQDFK55407827 = ssBthEQDFK98546634;     ssBthEQDFK98546634 = ssBthEQDFK80131635;     ssBthEQDFK80131635 = ssBthEQDFK15698597;     ssBthEQDFK15698597 = ssBthEQDFK42339170;     ssBthEQDFK42339170 = ssBthEQDFK43516497;     ssBthEQDFK43516497 = ssBthEQDFK34702140;     ssBthEQDFK34702140 = ssBthEQDFK11163806;     ssBthEQDFK11163806 = ssBthEQDFK88941594;     ssBthEQDFK88941594 = ssBthEQDFK17170007;     ssBthEQDFK17170007 = ssBthEQDFK2606198;     ssBthEQDFK2606198 = ssBthEQDFK85243584;     ssBthEQDFK85243584 = ssBthEQDFK96962628;     ssBthEQDFK96962628 = ssBthEQDFK17278531;     ssBthEQDFK17278531 = ssBthEQDFK93005277;     ssBthEQDFK93005277 = ssBthEQDFK72753381;     ssBthEQDFK72753381 = ssBthEQDFK39750219;     ssBthEQDFK39750219 = ssBthEQDFK43854490;     ssBthEQDFK43854490 = ssBthEQDFK75913173;     ssBthEQDFK75913173 = ssBthEQDFK18671335;     ssBthEQDFK18671335 = ssBthEQDFK80748775;     ssBthEQDFK80748775 = ssBthEQDFK31659593;     ssBthEQDFK31659593 = ssBthEQDFK94272357;     ssBthEQDFK94272357 = ssBthEQDFK83328269;     ssBthEQDFK83328269 = ssBthEQDFK39771499;     ssBthEQDFK39771499 = ssBthEQDFK41705866;     ssBthEQDFK41705866 = ssBthEQDFK66687790;     ssBthEQDFK66687790 = ssBthEQDFK49030467;     ssBthEQDFK49030467 = ssBthEQDFK19663187;     ssBthEQDFK19663187 = ssBthEQDFK4621254;     ssBthEQDFK4621254 = ssBthEQDFK346744;     ssBthEQDFK346744 = ssBthEQDFK36663551;     ssBthEQDFK36663551 = ssBthEQDFK52967305;     ssBthEQDFK52967305 = ssBthEQDFK72176813;     ssBthEQDFK72176813 = ssBthEQDFK70316996;     ssBthEQDFK70316996 = ssBthEQDFK86199649;     ssBthEQDFK86199649 = ssBthEQDFK77790088;     ssBthEQDFK77790088 = ssBthEQDFK48725414;     ssBthEQDFK48725414 = ssBthEQDFK27412476;     ssBthEQDFK27412476 = ssBthEQDFK94320644;     ssBthEQDFK94320644 = ssBthEQDFK64875120;     ssBthEQDFK64875120 = ssBthEQDFK91561728;     ssBthEQDFK91561728 = ssBthEQDFK95971901;     ssBthEQDFK95971901 = ssBthEQDFK25104886;     ssBthEQDFK25104886 = ssBthEQDFK31212944;     ssBthEQDFK31212944 = ssBthEQDFK13628605;     ssBthEQDFK13628605 = ssBthEQDFK52094266;     ssBthEQDFK52094266 = ssBthEQDFK96272882;     ssBthEQDFK96272882 = ssBthEQDFK7455686;     ssBthEQDFK7455686 = ssBthEQDFK7607647;     ssBthEQDFK7607647 = ssBthEQDFK93725609;     ssBthEQDFK93725609 = ssBthEQDFK68965646;     ssBthEQDFK68965646 = ssBthEQDFK16690901;     ssBthEQDFK16690901 = ssBthEQDFK88916825;     ssBthEQDFK88916825 = ssBthEQDFK71156481;     ssBthEQDFK71156481 = ssBthEQDFK23748235;     ssBthEQDFK23748235 = ssBthEQDFK4274277;     ssBthEQDFK4274277 = ssBthEQDFK96803366;     ssBthEQDFK96803366 = ssBthEQDFK75927098;     ssBthEQDFK75927098 = ssBthEQDFK633305;     ssBthEQDFK633305 = ssBthEQDFK76828707;     ssBthEQDFK76828707 = ssBthEQDFK85671672;     ssBthEQDFK85671672 = ssBthEQDFK91500619;     ssBthEQDFK91500619 = ssBthEQDFK84320341;     ssBthEQDFK84320341 = ssBthEQDFK16823264;     ssBthEQDFK16823264 = ssBthEQDFK65942646;     ssBthEQDFK65942646 = ssBthEQDFK32276279;     ssBthEQDFK32276279 = ssBthEQDFK24785815;     ssBthEQDFK24785815 = ssBthEQDFK46961534;     ssBthEQDFK46961534 = ssBthEQDFK6805628;     ssBthEQDFK6805628 = ssBthEQDFK94963292;     ssBthEQDFK94963292 = ssBthEQDFK91024804;     ssBthEQDFK91024804 = ssBthEQDFK16442015;     ssBthEQDFK16442015 = ssBthEQDFK81592529;     ssBthEQDFK81592529 = ssBthEQDFK53796214;     ssBthEQDFK53796214 = ssBthEQDFK89187047;     ssBthEQDFK89187047 = ssBthEQDFK35687692;     ssBthEQDFK35687692 = ssBthEQDFK69167471;     ssBthEQDFK69167471 = ssBthEQDFK52115325;     ssBthEQDFK52115325 = ssBthEQDFK26142894;     ssBthEQDFK26142894 = ssBthEQDFK89611600;     ssBthEQDFK89611600 = ssBthEQDFK70414907;     ssBthEQDFK70414907 = ssBthEQDFK64424011;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bHCccIQfis3617416() {     double twqVfzwJvR83612969 = -386452233;    double twqVfzwJvR42049504 = -508997399;    double twqVfzwJvR62496523 = -456994074;    double twqVfzwJvR83868706 = -660300102;    double twqVfzwJvR29732038 = -815492554;    double twqVfzwJvR97670453 = -961680481;    double twqVfzwJvR21265526 = -50479745;    double twqVfzwJvR70076734 = 51744973;    double twqVfzwJvR67869159 = -470988929;    double twqVfzwJvR13425681 = -360067206;    double twqVfzwJvR74301578 = -982039442;    double twqVfzwJvR76949205 = -763210789;    double twqVfzwJvR37796611 = -80774964;    double twqVfzwJvR27145616 = -109655296;    double twqVfzwJvR30021803 = -365737273;    double twqVfzwJvR73394855 = -98976450;    double twqVfzwJvR7436748 = -675141995;    double twqVfzwJvR42742185 = -669234746;    double twqVfzwJvR76506368 = -692354811;    double twqVfzwJvR94417734 = -130436568;    double twqVfzwJvR80888584 = 21149998;    double twqVfzwJvR41825927 = -482649055;    double twqVfzwJvR45162019 = -68981186;    double twqVfzwJvR94802028 = -39427882;    double twqVfzwJvR50879494 = -97862157;    double twqVfzwJvR58280658 = -837255823;    double twqVfzwJvR732527 = -563033126;    double twqVfzwJvR65094107 = -545582907;    double twqVfzwJvR8514107 = -483554413;    double twqVfzwJvR50235452 = -545507313;    double twqVfzwJvR63497604 = -892416825;    double twqVfzwJvR95170979 = -382720623;    double twqVfzwJvR33895566 = -389627114;    double twqVfzwJvR37943684 = -421556935;    double twqVfzwJvR90803960 = 58988867;    double twqVfzwJvR22448378 = -239300158;    double twqVfzwJvR13147213 = -277712279;    double twqVfzwJvR50350849 = 53327708;    double twqVfzwJvR36441460 = -838083797;    double twqVfzwJvR63247348 = -51993939;    double twqVfzwJvR4635819 = -69874845;    double twqVfzwJvR16335462 = -2458388;    double twqVfzwJvR1671664 = -472771988;    double twqVfzwJvR69800825 = -372569414;    double twqVfzwJvR10673448 = -478774459;    double twqVfzwJvR98830708 = -955619388;    double twqVfzwJvR44489527 = -319245137;    double twqVfzwJvR85705831 = -599366702;    double twqVfzwJvR38455233 = -919371845;    double twqVfzwJvR77021047 = -71619919;    double twqVfzwJvR60569491 = -791749619;    double twqVfzwJvR92620748 = -728264080;    double twqVfzwJvR8872200 = -383227664;    double twqVfzwJvR37873832 = -550963195;    double twqVfzwJvR22728832 = -574178665;    double twqVfzwJvR41787042 = -903803179;    double twqVfzwJvR96887485 = -340016214;    double twqVfzwJvR67694494 = -317566192;    double twqVfzwJvR32989213 = -462437945;    double twqVfzwJvR71451379 = -978236731;    double twqVfzwJvR96937927 = -298647356;    double twqVfzwJvR56171419 = -504896838;    double twqVfzwJvR61562628 = -464700614;    double twqVfzwJvR17633707 = -925481617;    double twqVfzwJvR49928077 = -467650382;    double twqVfzwJvR79130598 = -499318820;    double twqVfzwJvR43053639 = -273583675;    double twqVfzwJvR99852926 = -659218029;    double twqVfzwJvR36341656 = -68644164;    double twqVfzwJvR7573425 = -26437116;    double twqVfzwJvR60247643 = -821264172;    double twqVfzwJvR57085898 = -628469704;    double twqVfzwJvR6300726 = -831150950;    double twqVfzwJvR13259020 = -540360872;    double twqVfzwJvR89781915 = 39438276;    double twqVfzwJvR64553123 = -976391614;    double twqVfzwJvR40154264 = 90122933;    double twqVfzwJvR75361193 = -696411772;    double twqVfzwJvR84128581 = -560653423;    double twqVfzwJvR52048785 = -142242769;    double twqVfzwJvR13791132 = -418010687;    double twqVfzwJvR15026696 = -963666424;    double twqVfzwJvR26638874 = -626211063;    double twqVfzwJvR31493059 = -311934495;    double twqVfzwJvR89665961 = -753757695;    double twqVfzwJvR70876855 = -64152745;    double twqVfzwJvR86298780 = -999492959;    double twqVfzwJvR96021734 = -838663920;    double twqVfzwJvR15214853 = -847378271;    double twqVfzwJvR49016918 = -37207954;    double twqVfzwJvR25560893 = -899283945;    double twqVfzwJvR45452718 = -960146087;    double twqVfzwJvR17361637 = -484234347;    double twqVfzwJvR64990080 = -859847067;    double twqVfzwJvR66309421 = -753346584;    double twqVfzwJvR48464399 = -564978007;    double twqVfzwJvR54772834 = -537757775;    double twqVfzwJvR84037957 = -547290371;    double twqVfzwJvR19872749 = -904919033;    double twqVfzwJvR31542849 = -386452233;     twqVfzwJvR83612969 = twqVfzwJvR42049504;     twqVfzwJvR42049504 = twqVfzwJvR62496523;     twqVfzwJvR62496523 = twqVfzwJvR83868706;     twqVfzwJvR83868706 = twqVfzwJvR29732038;     twqVfzwJvR29732038 = twqVfzwJvR97670453;     twqVfzwJvR97670453 = twqVfzwJvR21265526;     twqVfzwJvR21265526 = twqVfzwJvR70076734;     twqVfzwJvR70076734 = twqVfzwJvR67869159;     twqVfzwJvR67869159 = twqVfzwJvR13425681;     twqVfzwJvR13425681 = twqVfzwJvR74301578;     twqVfzwJvR74301578 = twqVfzwJvR76949205;     twqVfzwJvR76949205 = twqVfzwJvR37796611;     twqVfzwJvR37796611 = twqVfzwJvR27145616;     twqVfzwJvR27145616 = twqVfzwJvR30021803;     twqVfzwJvR30021803 = twqVfzwJvR73394855;     twqVfzwJvR73394855 = twqVfzwJvR7436748;     twqVfzwJvR7436748 = twqVfzwJvR42742185;     twqVfzwJvR42742185 = twqVfzwJvR76506368;     twqVfzwJvR76506368 = twqVfzwJvR94417734;     twqVfzwJvR94417734 = twqVfzwJvR80888584;     twqVfzwJvR80888584 = twqVfzwJvR41825927;     twqVfzwJvR41825927 = twqVfzwJvR45162019;     twqVfzwJvR45162019 = twqVfzwJvR94802028;     twqVfzwJvR94802028 = twqVfzwJvR50879494;     twqVfzwJvR50879494 = twqVfzwJvR58280658;     twqVfzwJvR58280658 = twqVfzwJvR732527;     twqVfzwJvR732527 = twqVfzwJvR65094107;     twqVfzwJvR65094107 = twqVfzwJvR8514107;     twqVfzwJvR8514107 = twqVfzwJvR50235452;     twqVfzwJvR50235452 = twqVfzwJvR63497604;     twqVfzwJvR63497604 = twqVfzwJvR95170979;     twqVfzwJvR95170979 = twqVfzwJvR33895566;     twqVfzwJvR33895566 = twqVfzwJvR37943684;     twqVfzwJvR37943684 = twqVfzwJvR90803960;     twqVfzwJvR90803960 = twqVfzwJvR22448378;     twqVfzwJvR22448378 = twqVfzwJvR13147213;     twqVfzwJvR13147213 = twqVfzwJvR50350849;     twqVfzwJvR50350849 = twqVfzwJvR36441460;     twqVfzwJvR36441460 = twqVfzwJvR63247348;     twqVfzwJvR63247348 = twqVfzwJvR4635819;     twqVfzwJvR4635819 = twqVfzwJvR16335462;     twqVfzwJvR16335462 = twqVfzwJvR1671664;     twqVfzwJvR1671664 = twqVfzwJvR69800825;     twqVfzwJvR69800825 = twqVfzwJvR10673448;     twqVfzwJvR10673448 = twqVfzwJvR98830708;     twqVfzwJvR98830708 = twqVfzwJvR44489527;     twqVfzwJvR44489527 = twqVfzwJvR85705831;     twqVfzwJvR85705831 = twqVfzwJvR38455233;     twqVfzwJvR38455233 = twqVfzwJvR77021047;     twqVfzwJvR77021047 = twqVfzwJvR60569491;     twqVfzwJvR60569491 = twqVfzwJvR92620748;     twqVfzwJvR92620748 = twqVfzwJvR8872200;     twqVfzwJvR8872200 = twqVfzwJvR37873832;     twqVfzwJvR37873832 = twqVfzwJvR22728832;     twqVfzwJvR22728832 = twqVfzwJvR41787042;     twqVfzwJvR41787042 = twqVfzwJvR96887485;     twqVfzwJvR96887485 = twqVfzwJvR67694494;     twqVfzwJvR67694494 = twqVfzwJvR32989213;     twqVfzwJvR32989213 = twqVfzwJvR71451379;     twqVfzwJvR71451379 = twqVfzwJvR96937927;     twqVfzwJvR96937927 = twqVfzwJvR56171419;     twqVfzwJvR56171419 = twqVfzwJvR61562628;     twqVfzwJvR61562628 = twqVfzwJvR17633707;     twqVfzwJvR17633707 = twqVfzwJvR49928077;     twqVfzwJvR49928077 = twqVfzwJvR79130598;     twqVfzwJvR79130598 = twqVfzwJvR43053639;     twqVfzwJvR43053639 = twqVfzwJvR99852926;     twqVfzwJvR99852926 = twqVfzwJvR36341656;     twqVfzwJvR36341656 = twqVfzwJvR7573425;     twqVfzwJvR7573425 = twqVfzwJvR60247643;     twqVfzwJvR60247643 = twqVfzwJvR57085898;     twqVfzwJvR57085898 = twqVfzwJvR6300726;     twqVfzwJvR6300726 = twqVfzwJvR13259020;     twqVfzwJvR13259020 = twqVfzwJvR89781915;     twqVfzwJvR89781915 = twqVfzwJvR64553123;     twqVfzwJvR64553123 = twqVfzwJvR40154264;     twqVfzwJvR40154264 = twqVfzwJvR75361193;     twqVfzwJvR75361193 = twqVfzwJvR84128581;     twqVfzwJvR84128581 = twqVfzwJvR52048785;     twqVfzwJvR52048785 = twqVfzwJvR13791132;     twqVfzwJvR13791132 = twqVfzwJvR15026696;     twqVfzwJvR15026696 = twqVfzwJvR26638874;     twqVfzwJvR26638874 = twqVfzwJvR31493059;     twqVfzwJvR31493059 = twqVfzwJvR89665961;     twqVfzwJvR89665961 = twqVfzwJvR70876855;     twqVfzwJvR70876855 = twqVfzwJvR86298780;     twqVfzwJvR86298780 = twqVfzwJvR96021734;     twqVfzwJvR96021734 = twqVfzwJvR15214853;     twqVfzwJvR15214853 = twqVfzwJvR49016918;     twqVfzwJvR49016918 = twqVfzwJvR25560893;     twqVfzwJvR25560893 = twqVfzwJvR45452718;     twqVfzwJvR45452718 = twqVfzwJvR17361637;     twqVfzwJvR17361637 = twqVfzwJvR64990080;     twqVfzwJvR64990080 = twqVfzwJvR66309421;     twqVfzwJvR66309421 = twqVfzwJvR48464399;     twqVfzwJvR48464399 = twqVfzwJvR54772834;     twqVfzwJvR54772834 = twqVfzwJvR84037957;     twqVfzwJvR84037957 = twqVfzwJvR19872749;     twqVfzwJvR19872749 = twqVfzwJvR31542849;     twqVfzwJvR31542849 = twqVfzwJvR83612969;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MOIMByiFAr88350056() {     double RQZFohYPKO21997356 = -201477743;    double RQZFohYPKO48072902 = -849618939;    double RQZFohYPKO10502075 = -753557764;    double RQZFohYPKO67371740 = -850743611;    double RQZFohYPKO74904942 = -417520663;    double RQZFohYPKO88378936 = -394719174;    double RQZFohYPKO10699457 = -669350088;    double RQZFohYPKO73377870 = -265100528;    double RQZFohYPKO23122934 = -401678927;    double RQZFohYPKO45523741 = -125102620;    double RQZFohYPKO75481304 = -388209200;    double RQZFohYPKO15634013 = -43295787;    double RQZFohYPKO20814864 = -805489037;    double RQZFohYPKO60640971 = -826288685;    double RQZFohYPKO19253026 = -235843358;    double RQZFohYPKO60411873 = -420468015;    double RQZFohYPKO51281162 = -388007117;    double RQZFohYPKO1415509 = 53420835;    double RQZFohYPKO98275179 = -148833331;    double RQZFohYPKO14086171 = -580071180;    double RQZFohYPKO2733059 = -410933300;    double RQZFohYPKO15784322 = -52256566;    double RQZFohYPKO13929319 = -226161501;    double RQZFohYPKO53722984 = -496780829;    double RQZFohYPKO54791513 = -122412517;    double RQZFohYPKO35940286 = -275725707;    double RQZFohYPKO54092809 = -160953115;    double RQZFohYPKO25538782 = -831109268;    double RQZFohYPKO7651479 = -741924874;    double RQZFohYPKO4129319 = -733440282;    double RQZFohYPKO63154877 = -173294136;    double RQZFohYPKO66232021 = -951592612;    double RQZFohYPKO30223721 = -309414604;    double RQZFohYPKO68041454 = -629092660;    double RQZFohYPKO10553405 = -733851075;    double RQZFohYPKO36807964 = -385926485;    double RQZFohYPKO75673922 = -348826817;    double RQZFohYPKO43792200 = -824481851;    double RQZFohYPKO12513231 = -812909864;    double RQZFohYPKO54549966 = -47517732;    double RQZFohYPKO37507072 = -844521231;    double RQZFohYPKO96652185 = -22224062;    double RQZFohYPKO65188632 = -506551306;    double RQZFohYPKO66758846 = -968228369;    double RQZFohYPKO14626709 = -464046734;    double RQZFohYPKO58003224 = -827396023;    double RQZFohYPKO66444416 = -986667806;    double RQZFohYPKO72491407 = -830379616;    double RQZFohYPKO96013823 = -955122323;    double RQZFohYPKO37690617 = -256629762;    double RQZFohYPKO62514492 = 63817485;    double RQZFohYPKO18961361 = -613237101;    double RQZFohYPKO10534257 = -243021479;    double RQZFohYPKO30971924 = -491639016;    double RQZFohYPKO85616998 = -429915014;    double RQZFohYPKO6213035 = -49221178;    double RQZFohYPKO34143583 = -523457438;    double RQZFohYPKO56779090 = -156776936;    double RQZFohYPKO12580228 = -628331095;    double RQZFohYPKO38964656 = -41794956;    double RQZFohYPKO34286128 = -133766060;    double RQZFohYPKO85160674 = -838240821;    double RQZFohYPKO65726392 = -523175654;    double RQZFohYPKO18993615 = -668238645;    double RQZFohYPKO82368863 = -951808484;    double RQZFohYPKO9249283 = -436616589;    double RQZFohYPKO85410291 = -733881183;    double RQZFohYPKO52773410 = -76396378;    double RQZFohYPKO50087567 = 7562389;    double RQZFohYPKO82445061 = -849916873;    double RQZFohYPKO84737950 = 28358802;    double RQZFohYPKO7488963 = -563525267;    double RQZFohYPKO88902277 = -133669302;    double RQZFohYPKO43725214 = -1315599;    double RQZFohYPKO76579099 = -735549950;    double RQZFohYPKO6080873 = -288709238;    double RQZFohYPKO50595689 = -545705260;    double RQZFohYPKO47170473 = -257933133;    double RQZFohYPKO39096275 = 67265904;    double RQZFohYPKO96788289 = -295016494;    double RQZFohYPKO69495870 = -289057902;    double RQZFohYPKO81601401 = -330573499;    double RQZFohYPKO29524958 = -875986945;    double RQZFohYPKO69960861 = -385295112;    double RQZFohYPKO41614827 = -697257768;    double RQZFohYPKO44193517 = -560057036;    double RQZFohYPKO55697765 = -608571133;    double RQZFohYPKO99251797 = -817775588;    double RQZFohYPKO82424455 = -99177647;    double RQZFohYPKO4340370 = -584629898;    double RQZFohYPKO2664381 = -862469048;    double RQZFohYPKO18894833 = -92049882;    double RQZFohYPKO31211973 = -96150756;    double RQZFohYPKO73548574 = -671114908;    double RQZFohYPKO20263839 = -913751673;    double RQZFohYPKO52346397 = 93719589;    double RQZFohYPKO30925794 = -499048408;    double RQZFohYPKO46195017 = -838312661;    double RQZFohYPKO84389982 = 83580115;    double RQZFohYPKO5377427 = -201477743;     RQZFohYPKO21997356 = RQZFohYPKO48072902;     RQZFohYPKO48072902 = RQZFohYPKO10502075;     RQZFohYPKO10502075 = RQZFohYPKO67371740;     RQZFohYPKO67371740 = RQZFohYPKO74904942;     RQZFohYPKO74904942 = RQZFohYPKO88378936;     RQZFohYPKO88378936 = RQZFohYPKO10699457;     RQZFohYPKO10699457 = RQZFohYPKO73377870;     RQZFohYPKO73377870 = RQZFohYPKO23122934;     RQZFohYPKO23122934 = RQZFohYPKO45523741;     RQZFohYPKO45523741 = RQZFohYPKO75481304;     RQZFohYPKO75481304 = RQZFohYPKO15634013;     RQZFohYPKO15634013 = RQZFohYPKO20814864;     RQZFohYPKO20814864 = RQZFohYPKO60640971;     RQZFohYPKO60640971 = RQZFohYPKO19253026;     RQZFohYPKO19253026 = RQZFohYPKO60411873;     RQZFohYPKO60411873 = RQZFohYPKO51281162;     RQZFohYPKO51281162 = RQZFohYPKO1415509;     RQZFohYPKO1415509 = RQZFohYPKO98275179;     RQZFohYPKO98275179 = RQZFohYPKO14086171;     RQZFohYPKO14086171 = RQZFohYPKO2733059;     RQZFohYPKO2733059 = RQZFohYPKO15784322;     RQZFohYPKO15784322 = RQZFohYPKO13929319;     RQZFohYPKO13929319 = RQZFohYPKO53722984;     RQZFohYPKO53722984 = RQZFohYPKO54791513;     RQZFohYPKO54791513 = RQZFohYPKO35940286;     RQZFohYPKO35940286 = RQZFohYPKO54092809;     RQZFohYPKO54092809 = RQZFohYPKO25538782;     RQZFohYPKO25538782 = RQZFohYPKO7651479;     RQZFohYPKO7651479 = RQZFohYPKO4129319;     RQZFohYPKO4129319 = RQZFohYPKO63154877;     RQZFohYPKO63154877 = RQZFohYPKO66232021;     RQZFohYPKO66232021 = RQZFohYPKO30223721;     RQZFohYPKO30223721 = RQZFohYPKO68041454;     RQZFohYPKO68041454 = RQZFohYPKO10553405;     RQZFohYPKO10553405 = RQZFohYPKO36807964;     RQZFohYPKO36807964 = RQZFohYPKO75673922;     RQZFohYPKO75673922 = RQZFohYPKO43792200;     RQZFohYPKO43792200 = RQZFohYPKO12513231;     RQZFohYPKO12513231 = RQZFohYPKO54549966;     RQZFohYPKO54549966 = RQZFohYPKO37507072;     RQZFohYPKO37507072 = RQZFohYPKO96652185;     RQZFohYPKO96652185 = RQZFohYPKO65188632;     RQZFohYPKO65188632 = RQZFohYPKO66758846;     RQZFohYPKO66758846 = RQZFohYPKO14626709;     RQZFohYPKO14626709 = RQZFohYPKO58003224;     RQZFohYPKO58003224 = RQZFohYPKO66444416;     RQZFohYPKO66444416 = RQZFohYPKO72491407;     RQZFohYPKO72491407 = RQZFohYPKO96013823;     RQZFohYPKO96013823 = RQZFohYPKO37690617;     RQZFohYPKO37690617 = RQZFohYPKO62514492;     RQZFohYPKO62514492 = RQZFohYPKO18961361;     RQZFohYPKO18961361 = RQZFohYPKO10534257;     RQZFohYPKO10534257 = RQZFohYPKO30971924;     RQZFohYPKO30971924 = RQZFohYPKO85616998;     RQZFohYPKO85616998 = RQZFohYPKO6213035;     RQZFohYPKO6213035 = RQZFohYPKO34143583;     RQZFohYPKO34143583 = RQZFohYPKO56779090;     RQZFohYPKO56779090 = RQZFohYPKO12580228;     RQZFohYPKO12580228 = RQZFohYPKO38964656;     RQZFohYPKO38964656 = RQZFohYPKO34286128;     RQZFohYPKO34286128 = RQZFohYPKO85160674;     RQZFohYPKO85160674 = RQZFohYPKO65726392;     RQZFohYPKO65726392 = RQZFohYPKO18993615;     RQZFohYPKO18993615 = RQZFohYPKO82368863;     RQZFohYPKO82368863 = RQZFohYPKO9249283;     RQZFohYPKO9249283 = RQZFohYPKO85410291;     RQZFohYPKO85410291 = RQZFohYPKO52773410;     RQZFohYPKO52773410 = RQZFohYPKO50087567;     RQZFohYPKO50087567 = RQZFohYPKO82445061;     RQZFohYPKO82445061 = RQZFohYPKO84737950;     RQZFohYPKO84737950 = RQZFohYPKO7488963;     RQZFohYPKO7488963 = RQZFohYPKO88902277;     RQZFohYPKO88902277 = RQZFohYPKO43725214;     RQZFohYPKO43725214 = RQZFohYPKO76579099;     RQZFohYPKO76579099 = RQZFohYPKO6080873;     RQZFohYPKO6080873 = RQZFohYPKO50595689;     RQZFohYPKO50595689 = RQZFohYPKO47170473;     RQZFohYPKO47170473 = RQZFohYPKO39096275;     RQZFohYPKO39096275 = RQZFohYPKO96788289;     RQZFohYPKO96788289 = RQZFohYPKO69495870;     RQZFohYPKO69495870 = RQZFohYPKO81601401;     RQZFohYPKO81601401 = RQZFohYPKO29524958;     RQZFohYPKO29524958 = RQZFohYPKO69960861;     RQZFohYPKO69960861 = RQZFohYPKO41614827;     RQZFohYPKO41614827 = RQZFohYPKO44193517;     RQZFohYPKO44193517 = RQZFohYPKO55697765;     RQZFohYPKO55697765 = RQZFohYPKO99251797;     RQZFohYPKO99251797 = RQZFohYPKO82424455;     RQZFohYPKO82424455 = RQZFohYPKO4340370;     RQZFohYPKO4340370 = RQZFohYPKO2664381;     RQZFohYPKO2664381 = RQZFohYPKO18894833;     RQZFohYPKO18894833 = RQZFohYPKO31211973;     RQZFohYPKO31211973 = RQZFohYPKO73548574;     RQZFohYPKO73548574 = RQZFohYPKO20263839;     RQZFohYPKO20263839 = RQZFohYPKO52346397;     RQZFohYPKO52346397 = RQZFohYPKO30925794;     RQZFohYPKO30925794 = RQZFohYPKO46195017;     RQZFohYPKO46195017 = RQZFohYPKO84389982;     RQZFohYPKO84389982 = RQZFohYPKO5377427;     RQZFohYPKO5377427 = RQZFohYPKO21997356;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LcwhjLyylG95008801() {     double wvzfKGqVXW21990885 = -74001886;    double wvzfKGqVXW86645218 = -892214070;    double wvzfKGqVXW11815287 = -710686378;    double wvzfKGqVXW52451601 = -671284438;    double wvzfKGqVXW83745044 = -885875993;    double wvzfKGqVXW23184369 = -94151986;    double wvzfKGqVXW79246792 = -373886555;    double wvzfKGqVXW42591921 = 30722139;    double wvzfKGqVXW5863098 = -115324997;    double wvzfKGqVXW42163254 = -861909491;    double wvzfKGqVXW95801909 = -347681273;    double wvzfKGqVXW29925690 = -878774158;    double wvzfKGqVXW52667778 = -827147512;    double wvzfKGqVXW14485192 = -71441738;    double wvzfKGqVXW64425134 = -198556228;    double wvzfKGqVXW34976421 = -753263665;    double wvzfKGqVXW7133922 = -207621687;    double wvzfKGqVXW24416041 = -343650791;    double wvzfKGqVXW18703451 = -766815596;    double wvzfKGqVXW86159934 = -92095632;    double wvzfKGqVXW82402482 = -121399679;    double wvzfKGqVXW94080587 = -562248449;    double wvzfKGqVXW50807716 = -685119343;    double wvzfKGqVXW97373092 = -377701352;    double wvzfKGqVXW83234202 = -549106383;    double wvzfKGqVXW52514364 = -183931632;    double wvzfKGqVXW24314390 = -461752828;    double wvzfKGqVXW60942307 = -340076263;    double wvzfKGqVXW20329926 = -287995174;    double wvzfKGqVXW80219188 = -408871656;    double wvzfKGqVXW96567555 = -560387371;    double wvzfKGqVXW50955879 = -243414370;    double wvzfKGqVXW15676144 = -511358817;    double wvzfKGqVXW68324291 = -505774393;    double wvzfKGqVXW92911442 = -993164997;    double wvzfKGqVXW79636154 = -465692213;    double wvzfKGqVXW87615290 = -448480195;    double wvzfKGqVXW13709880 = -819744533;    double wvzfKGqVXW47826828 = -425591268;    double wvzfKGqVXW26422859 = -756949418;    double wvzfKGqVXW25362740 = -932465129;    double wvzfKGqVXW82349846 = -216016124;    double wvzfKGqVXW25471993 = 34318442;    double wvzfKGqVXW25990745 = -525192813;    double wvzfKGqVXW98644761 = -937306134;    double wvzfKGqVXW98431191 = -639440676;    double wvzfKGqVXW94142207 = -367948428;    double wvzfKGqVXW47874988 = -339140971;    double wvzfKGqVXW14672214 = -518886160;    double wvzfKGqVXW57736040 = -943654654;    double wvzfKGqVXW75773862 = -614737522;    double wvzfKGqVXW33508618 = -995382845;    double wvzfKGqVXW85982604 = -135075490;    double wvzfKGqVXW41222195 = 76441133;    double wvzfKGqVXW12606319 = -186814286;    double wvzfKGqVXW27910298 = -511753438;    double wvzfKGqVXW35837503 = -107094727;    double wvzfKGqVXW14442194 = -232985027;    double wvzfKGqVXW69217398 = -22178056;    double wvzfKGqVXW31230681 = -601944362;    double wvzfKGqVXW98869978 = -632399159;    double wvzfKGqVXW18304486 = 66189707;    double wvzfKGqVXW22261996 = -681282687;    double wvzfKGqVXW25643909 = -706453342;    double wvzfKGqVXW45595699 = -201522121;    double wvzfKGqVXW44846031 = -4266903;    double wvzfKGqVXW14249546 = -267415342;    double wvzfKGqVXW84343486 = -221373120;    double wvzfKGqVXW21573750 = -78276741;    double wvzfKGqVXW84788980 = -732864016;    double wvzfKGqVXW47361130 = -204783470;    double wvzfKGqVXW93424042 = -387877154;    double wvzfKGqVXW76589213 = -918059524;    double wvzfKGqVXW92280591 = 90133821;    double wvzfKGqVXW60797194 = -159630503;    double wvzfKGqVXW52636 = -905383555;    double wvzfKGqVXW68608595 = -496566891;    double wvzfKGqVXW24816972 = -59926531;    double wvzfKGqVXW98728331 = -440395218;    double wvzfKGqVXW84803010 = -909665707;    double wvzfKGqVXW58372156 = -815983204;    double wvzfKGqVXW76439402 = -22611857;    double wvzfKGqVXW46270094 = -821190103;    double wvzfKGqVXW62593885 = -344340520;    double wvzfKGqVXW4445327 = -794134134;    double wvzfKGqVXW63058938 = -565004526;    double wvzfKGqVXW64973274 = -8338880;    double wvzfKGqVXW74453949 = -487799950;    double wvzfKGqVXW55717972 = -218960107;    double wvzfKGqVXW65001145 = -381411559;    double wvzfKGqVXW43798652 = -258597487;    double wvzfKGqVXW73173096 = -115495169;    double wvzfKGqVXW44492481 = -697566477;    double wvzfKGqVXW16596147 = -823646907;    double wvzfKGqVXW27552880 = -24550260;    double wvzfKGqVXW7058254 = -898654837;    double wvzfKGqVXW60087851 = -534733437;    double wvzfKGqVXW99828083 = -259228216;    double wvzfKGqVXW80395045 = -223670692;    double wvzfKGqVXW53798731 = -74001886;     wvzfKGqVXW21990885 = wvzfKGqVXW86645218;     wvzfKGqVXW86645218 = wvzfKGqVXW11815287;     wvzfKGqVXW11815287 = wvzfKGqVXW52451601;     wvzfKGqVXW52451601 = wvzfKGqVXW83745044;     wvzfKGqVXW83745044 = wvzfKGqVXW23184369;     wvzfKGqVXW23184369 = wvzfKGqVXW79246792;     wvzfKGqVXW79246792 = wvzfKGqVXW42591921;     wvzfKGqVXW42591921 = wvzfKGqVXW5863098;     wvzfKGqVXW5863098 = wvzfKGqVXW42163254;     wvzfKGqVXW42163254 = wvzfKGqVXW95801909;     wvzfKGqVXW95801909 = wvzfKGqVXW29925690;     wvzfKGqVXW29925690 = wvzfKGqVXW52667778;     wvzfKGqVXW52667778 = wvzfKGqVXW14485192;     wvzfKGqVXW14485192 = wvzfKGqVXW64425134;     wvzfKGqVXW64425134 = wvzfKGqVXW34976421;     wvzfKGqVXW34976421 = wvzfKGqVXW7133922;     wvzfKGqVXW7133922 = wvzfKGqVXW24416041;     wvzfKGqVXW24416041 = wvzfKGqVXW18703451;     wvzfKGqVXW18703451 = wvzfKGqVXW86159934;     wvzfKGqVXW86159934 = wvzfKGqVXW82402482;     wvzfKGqVXW82402482 = wvzfKGqVXW94080587;     wvzfKGqVXW94080587 = wvzfKGqVXW50807716;     wvzfKGqVXW50807716 = wvzfKGqVXW97373092;     wvzfKGqVXW97373092 = wvzfKGqVXW83234202;     wvzfKGqVXW83234202 = wvzfKGqVXW52514364;     wvzfKGqVXW52514364 = wvzfKGqVXW24314390;     wvzfKGqVXW24314390 = wvzfKGqVXW60942307;     wvzfKGqVXW60942307 = wvzfKGqVXW20329926;     wvzfKGqVXW20329926 = wvzfKGqVXW80219188;     wvzfKGqVXW80219188 = wvzfKGqVXW96567555;     wvzfKGqVXW96567555 = wvzfKGqVXW50955879;     wvzfKGqVXW50955879 = wvzfKGqVXW15676144;     wvzfKGqVXW15676144 = wvzfKGqVXW68324291;     wvzfKGqVXW68324291 = wvzfKGqVXW92911442;     wvzfKGqVXW92911442 = wvzfKGqVXW79636154;     wvzfKGqVXW79636154 = wvzfKGqVXW87615290;     wvzfKGqVXW87615290 = wvzfKGqVXW13709880;     wvzfKGqVXW13709880 = wvzfKGqVXW47826828;     wvzfKGqVXW47826828 = wvzfKGqVXW26422859;     wvzfKGqVXW26422859 = wvzfKGqVXW25362740;     wvzfKGqVXW25362740 = wvzfKGqVXW82349846;     wvzfKGqVXW82349846 = wvzfKGqVXW25471993;     wvzfKGqVXW25471993 = wvzfKGqVXW25990745;     wvzfKGqVXW25990745 = wvzfKGqVXW98644761;     wvzfKGqVXW98644761 = wvzfKGqVXW98431191;     wvzfKGqVXW98431191 = wvzfKGqVXW94142207;     wvzfKGqVXW94142207 = wvzfKGqVXW47874988;     wvzfKGqVXW47874988 = wvzfKGqVXW14672214;     wvzfKGqVXW14672214 = wvzfKGqVXW57736040;     wvzfKGqVXW57736040 = wvzfKGqVXW75773862;     wvzfKGqVXW75773862 = wvzfKGqVXW33508618;     wvzfKGqVXW33508618 = wvzfKGqVXW85982604;     wvzfKGqVXW85982604 = wvzfKGqVXW41222195;     wvzfKGqVXW41222195 = wvzfKGqVXW12606319;     wvzfKGqVXW12606319 = wvzfKGqVXW27910298;     wvzfKGqVXW27910298 = wvzfKGqVXW35837503;     wvzfKGqVXW35837503 = wvzfKGqVXW14442194;     wvzfKGqVXW14442194 = wvzfKGqVXW69217398;     wvzfKGqVXW69217398 = wvzfKGqVXW31230681;     wvzfKGqVXW31230681 = wvzfKGqVXW98869978;     wvzfKGqVXW98869978 = wvzfKGqVXW18304486;     wvzfKGqVXW18304486 = wvzfKGqVXW22261996;     wvzfKGqVXW22261996 = wvzfKGqVXW25643909;     wvzfKGqVXW25643909 = wvzfKGqVXW45595699;     wvzfKGqVXW45595699 = wvzfKGqVXW44846031;     wvzfKGqVXW44846031 = wvzfKGqVXW14249546;     wvzfKGqVXW14249546 = wvzfKGqVXW84343486;     wvzfKGqVXW84343486 = wvzfKGqVXW21573750;     wvzfKGqVXW21573750 = wvzfKGqVXW84788980;     wvzfKGqVXW84788980 = wvzfKGqVXW47361130;     wvzfKGqVXW47361130 = wvzfKGqVXW93424042;     wvzfKGqVXW93424042 = wvzfKGqVXW76589213;     wvzfKGqVXW76589213 = wvzfKGqVXW92280591;     wvzfKGqVXW92280591 = wvzfKGqVXW60797194;     wvzfKGqVXW60797194 = wvzfKGqVXW52636;     wvzfKGqVXW52636 = wvzfKGqVXW68608595;     wvzfKGqVXW68608595 = wvzfKGqVXW24816972;     wvzfKGqVXW24816972 = wvzfKGqVXW98728331;     wvzfKGqVXW98728331 = wvzfKGqVXW84803010;     wvzfKGqVXW84803010 = wvzfKGqVXW58372156;     wvzfKGqVXW58372156 = wvzfKGqVXW76439402;     wvzfKGqVXW76439402 = wvzfKGqVXW46270094;     wvzfKGqVXW46270094 = wvzfKGqVXW62593885;     wvzfKGqVXW62593885 = wvzfKGqVXW4445327;     wvzfKGqVXW4445327 = wvzfKGqVXW63058938;     wvzfKGqVXW63058938 = wvzfKGqVXW64973274;     wvzfKGqVXW64973274 = wvzfKGqVXW74453949;     wvzfKGqVXW74453949 = wvzfKGqVXW55717972;     wvzfKGqVXW55717972 = wvzfKGqVXW65001145;     wvzfKGqVXW65001145 = wvzfKGqVXW43798652;     wvzfKGqVXW43798652 = wvzfKGqVXW73173096;     wvzfKGqVXW73173096 = wvzfKGqVXW44492481;     wvzfKGqVXW44492481 = wvzfKGqVXW16596147;     wvzfKGqVXW16596147 = wvzfKGqVXW27552880;     wvzfKGqVXW27552880 = wvzfKGqVXW7058254;     wvzfKGqVXW7058254 = wvzfKGqVXW60087851;     wvzfKGqVXW60087851 = wvzfKGqVXW99828083;     wvzfKGqVXW99828083 = wvzfKGqVXW80395045;     wvzfKGqVXW80395045 = wvzfKGqVXW53798731;     wvzfKGqVXW53798731 = wvzfKGqVXW21990885;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mvjDukSuPs40704494() {     double aKJyRuIGKV41179843 = 82223288;    double aKJyRuIGKV58943076 = -533822403;    double aKJyRuIGKV36474668 = -287532530;    double aKJyRuIGKV36743048 = -676776608;    double aKJyRuIGKV10751549 = -921067708;    double aKJyRuIGKV35941326 = -760387739;    double aKJyRuIGKV58237426 = -535589961;    double aKJyRuIGKV78849514 = -529789270;    double aKJyRuIGKV74860067 = -487493031;    double aKJyRuIGKV6532042 = -12830634;    double aKJyRuIGKV56552075 = -30502188;    double aKJyRuIGKV6413932 = -936555838;    double aKJyRuIGKV10103361 = -100333786;    double aKJyRuIGKV8154980 = -52334958;    double aKJyRuIGKV81626800 = -664965697;    double aKJyRuIGKV15767203 = 19592727;    double aKJyRuIGKV6982509 = 26138468;    double aKJyRuIGKV15252970 = -730858812;    double aKJyRuIGKV39801991 = -254045989;    double aKJyRuIGKV82031034 = -622925163;    double aKJyRuIGKV83159431 = -742674514;    double aKJyRuIGKV20207918 = -52048146;    double aKJyRuIGKV53630564 = -993188422;    double aKJyRuIGKV48658624 = -546838086;    double aKJyRuIGKV99411557 = -224728489;    double aKJyRuIGKV99631216 = -407269538;    double aKJyRuIGKV36105322 = -411112679;    double aKJyRuIGKV8866407 = -787322937;    double aKJyRuIGKV26237835 = -740215562;    double aKJyRuIGKV45211057 = -340553827;    double aKJyRuIGKV63102531 = -944372644;    double aKJyRuIGKV28848328 = -173761246;    double aKJyRuIGKV56566433 = -22224670;    double aKJyRuIGKV33514595 = 2116879;    double aKJyRuIGKV43965183 = -419241930;    double aKJyRuIGKV58230042 = -578888250;    double aKJyRuIGKV24849330 = -533864154;    double aKJyRuIGKV45389394 = -706280651;    double aKJyRuIGKV3519512 = -769345009;    double aKJyRuIGKV58010614 = -559427158;    double aKJyRuIGKV35726201 = -263760272;    double aKJyRuIGKV15357040 = -322795002;    double aKJyRuIGKV87372157 = -812136344;    double aKJyRuIGKV54085704 = -51504512;    double aKJyRuIGKV42630418 = -616571972;    double aKJyRuIGKV48231434 = -481351329;    double aKJyRuIGKV18968549 = -392300073;    double aKJyRuIGKV28959566 = -759028106;    double aKJyRuIGKV52780703 = -318643330;    double aKJyRuIGKV98093536 = -279672019;    double aKJyRuIGKV83376047 = 23768526;    double aKJyRuIGKV53952552 = -578942230;    double aKJyRuIGKV24537807 = -10999402;    double aKJyRuIGKV92896376 = -159856703;    double aKJyRuIGKV57545062 = 6867901;    double aKJyRuIGKV20971925 = -865728566;    double aKJyRuIGKV5312512 = -540633981;    double aKJyRuIGKV87816043 = -740694444;    double aKJyRuIGKV37331491 = -352048120;    double aKJyRuIGKV11120332 = -413798171;    double aKJyRuIGKV99836004 = -249275060;    double aKJyRuIGKV49371019 = -748267024;    double aKJyRuIGKV52611679 = -789573709;    double aKJyRuIGKV29649011 = -46939204;    double aKJyRuIGKV43429510 = -68457990;    double aKJyRuIGKV27703747 = -856740943;    double aKJyRuIGKV49847498 = -814331169;    double aKJyRuIGKV76588766 = -2450665;    double aKJyRuIGKV64189796 = -633093028;    double aKJyRuIGKV23396759 = 13922553;    double aKJyRuIGKV90917873 = -446543120;    double aKJyRuIGKV61593114 = -267580881;    double aKJyRuIGKV11733458 = -961513803;    double aKJyRuIGKV81791377 = -694618832;    double aKJyRuIGKV46304834 = -259164892;    double aKJyRuIGKV67802392 = -319879513;    double aKJyRuIGKV32835761 = -239911802;    double aKJyRuIGKV99544860 = -841683911;    double aKJyRuIGKV6028206 = -930266115;    double aKJyRuIGKV51180124 = -743377161;    double aKJyRuIGKV80662668 = 85030535;    double aKJyRuIGKV7145757 = -652084573;    double aKJyRuIGKV56085704 = -368679607;    double aKJyRuIGKV28144299 = -360543543;    double aKJyRuIGKV61835009 = -264322354;    double aKJyRuIGKV9149979 = -265430415;    double aKJyRuIGKV4310521 = -62761844;    double aKJyRuIGKV63670057 = -862367967;    double aKJyRuIGKV75969532 = 95248978;    double aKJyRuIGKV22993258 = -553513364;    double aKJyRuIGKV52917530 = 61745730;    double aKJyRuIGKV37033286 = -793169710;    double aKJyRuIGKV8057904 = -254232532;    double aKJyRuIGKV92399179 = -255546839;    double aKJyRuIGKV58174610 = -210152098;    double aKJyRuIGKV86355181 = -515493248;    double aKJyRuIGKV62745360 = -533221294;    double aKJyRuIGKV57723146 = -665197140;    double aKJyRuIGKV10656194 = -983046522;    double aKJyRuIGKV14926672 = 82223288;     aKJyRuIGKV41179843 = aKJyRuIGKV58943076;     aKJyRuIGKV58943076 = aKJyRuIGKV36474668;     aKJyRuIGKV36474668 = aKJyRuIGKV36743048;     aKJyRuIGKV36743048 = aKJyRuIGKV10751549;     aKJyRuIGKV10751549 = aKJyRuIGKV35941326;     aKJyRuIGKV35941326 = aKJyRuIGKV58237426;     aKJyRuIGKV58237426 = aKJyRuIGKV78849514;     aKJyRuIGKV78849514 = aKJyRuIGKV74860067;     aKJyRuIGKV74860067 = aKJyRuIGKV6532042;     aKJyRuIGKV6532042 = aKJyRuIGKV56552075;     aKJyRuIGKV56552075 = aKJyRuIGKV6413932;     aKJyRuIGKV6413932 = aKJyRuIGKV10103361;     aKJyRuIGKV10103361 = aKJyRuIGKV8154980;     aKJyRuIGKV8154980 = aKJyRuIGKV81626800;     aKJyRuIGKV81626800 = aKJyRuIGKV15767203;     aKJyRuIGKV15767203 = aKJyRuIGKV6982509;     aKJyRuIGKV6982509 = aKJyRuIGKV15252970;     aKJyRuIGKV15252970 = aKJyRuIGKV39801991;     aKJyRuIGKV39801991 = aKJyRuIGKV82031034;     aKJyRuIGKV82031034 = aKJyRuIGKV83159431;     aKJyRuIGKV83159431 = aKJyRuIGKV20207918;     aKJyRuIGKV20207918 = aKJyRuIGKV53630564;     aKJyRuIGKV53630564 = aKJyRuIGKV48658624;     aKJyRuIGKV48658624 = aKJyRuIGKV99411557;     aKJyRuIGKV99411557 = aKJyRuIGKV99631216;     aKJyRuIGKV99631216 = aKJyRuIGKV36105322;     aKJyRuIGKV36105322 = aKJyRuIGKV8866407;     aKJyRuIGKV8866407 = aKJyRuIGKV26237835;     aKJyRuIGKV26237835 = aKJyRuIGKV45211057;     aKJyRuIGKV45211057 = aKJyRuIGKV63102531;     aKJyRuIGKV63102531 = aKJyRuIGKV28848328;     aKJyRuIGKV28848328 = aKJyRuIGKV56566433;     aKJyRuIGKV56566433 = aKJyRuIGKV33514595;     aKJyRuIGKV33514595 = aKJyRuIGKV43965183;     aKJyRuIGKV43965183 = aKJyRuIGKV58230042;     aKJyRuIGKV58230042 = aKJyRuIGKV24849330;     aKJyRuIGKV24849330 = aKJyRuIGKV45389394;     aKJyRuIGKV45389394 = aKJyRuIGKV3519512;     aKJyRuIGKV3519512 = aKJyRuIGKV58010614;     aKJyRuIGKV58010614 = aKJyRuIGKV35726201;     aKJyRuIGKV35726201 = aKJyRuIGKV15357040;     aKJyRuIGKV15357040 = aKJyRuIGKV87372157;     aKJyRuIGKV87372157 = aKJyRuIGKV54085704;     aKJyRuIGKV54085704 = aKJyRuIGKV42630418;     aKJyRuIGKV42630418 = aKJyRuIGKV48231434;     aKJyRuIGKV48231434 = aKJyRuIGKV18968549;     aKJyRuIGKV18968549 = aKJyRuIGKV28959566;     aKJyRuIGKV28959566 = aKJyRuIGKV52780703;     aKJyRuIGKV52780703 = aKJyRuIGKV98093536;     aKJyRuIGKV98093536 = aKJyRuIGKV83376047;     aKJyRuIGKV83376047 = aKJyRuIGKV53952552;     aKJyRuIGKV53952552 = aKJyRuIGKV24537807;     aKJyRuIGKV24537807 = aKJyRuIGKV92896376;     aKJyRuIGKV92896376 = aKJyRuIGKV57545062;     aKJyRuIGKV57545062 = aKJyRuIGKV20971925;     aKJyRuIGKV20971925 = aKJyRuIGKV5312512;     aKJyRuIGKV5312512 = aKJyRuIGKV87816043;     aKJyRuIGKV87816043 = aKJyRuIGKV37331491;     aKJyRuIGKV37331491 = aKJyRuIGKV11120332;     aKJyRuIGKV11120332 = aKJyRuIGKV99836004;     aKJyRuIGKV99836004 = aKJyRuIGKV49371019;     aKJyRuIGKV49371019 = aKJyRuIGKV52611679;     aKJyRuIGKV52611679 = aKJyRuIGKV29649011;     aKJyRuIGKV29649011 = aKJyRuIGKV43429510;     aKJyRuIGKV43429510 = aKJyRuIGKV27703747;     aKJyRuIGKV27703747 = aKJyRuIGKV49847498;     aKJyRuIGKV49847498 = aKJyRuIGKV76588766;     aKJyRuIGKV76588766 = aKJyRuIGKV64189796;     aKJyRuIGKV64189796 = aKJyRuIGKV23396759;     aKJyRuIGKV23396759 = aKJyRuIGKV90917873;     aKJyRuIGKV90917873 = aKJyRuIGKV61593114;     aKJyRuIGKV61593114 = aKJyRuIGKV11733458;     aKJyRuIGKV11733458 = aKJyRuIGKV81791377;     aKJyRuIGKV81791377 = aKJyRuIGKV46304834;     aKJyRuIGKV46304834 = aKJyRuIGKV67802392;     aKJyRuIGKV67802392 = aKJyRuIGKV32835761;     aKJyRuIGKV32835761 = aKJyRuIGKV99544860;     aKJyRuIGKV99544860 = aKJyRuIGKV6028206;     aKJyRuIGKV6028206 = aKJyRuIGKV51180124;     aKJyRuIGKV51180124 = aKJyRuIGKV80662668;     aKJyRuIGKV80662668 = aKJyRuIGKV7145757;     aKJyRuIGKV7145757 = aKJyRuIGKV56085704;     aKJyRuIGKV56085704 = aKJyRuIGKV28144299;     aKJyRuIGKV28144299 = aKJyRuIGKV61835009;     aKJyRuIGKV61835009 = aKJyRuIGKV9149979;     aKJyRuIGKV9149979 = aKJyRuIGKV4310521;     aKJyRuIGKV4310521 = aKJyRuIGKV63670057;     aKJyRuIGKV63670057 = aKJyRuIGKV75969532;     aKJyRuIGKV75969532 = aKJyRuIGKV22993258;     aKJyRuIGKV22993258 = aKJyRuIGKV52917530;     aKJyRuIGKV52917530 = aKJyRuIGKV37033286;     aKJyRuIGKV37033286 = aKJyRuIGKV8057904;     aKJyRuIGKV8057904 = aKJyRuIGKV92399179;     aKJyRuIGKV92399179 = aKJyRuIGKV58174610;     aKJyRuIGKV58174610 = aKJyRuIGKV86355181;     aKJyRuIGKV86355181 = aKJyRuIGKV62745360;     aKJyRuIGKV62745360 = aKJyRuIGKV57723146;     aKJyRuIGKV57723146 = aKJyRuIGKV10656194;     aKJyRuIGKV10656194 = aKJyRuIGKV14926672;     aKJyRuIGKV14926672 = aKJyRuIGKV41179843;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZfeIVNqtXE95120706() {     double PpRRgYMhyx11831510 = -535925842;    double PpRRgYMhyx27616593 = -405465702;    double PpRRgYMhyx16443588 = -729826990;    double PpRRgYMhyx76674335 = -866914998;    double PpRRgYMhyx32201870 = -704474056;    double PpRRgYMhyx25941089 = -461968890;    double PpRRgYMhyx76616321 = -534365671;    double PpRRgYMhyx35691895 = -876606360;    double PpRRgYMhyx20725122 = -580840360;    double PpRRgYMhyx29498502 = -313925984;    double PpRRgYMhyx98801235 = -65404118;    double PpRRgYMhyx51960502 = -152319631;    double PpRRgYMhyx89930748 = -9870844;    double PpRRgYMhyx92002012 = -953363168;    double PpRRgYMhyx64346820 = -814715700;    double PpRRgYMhyx48295843 = -650390860;    double PpRRgYMhyx835336 = -983046662;    double PpRRgYMhyx35546464 = -720025008;    double PpRRgYMhyx21509773 = -961233931;    double PpRRgYMhyx40817743 = -65291468;    double PpRRgYMhyx38295186 = -651353655;    double PpRRgYMhyx81603682 = -383333451;    double PpRRgYMhyx55574373 = -461031567;    double PpRRgYMhyx82508161 = -139238993;    double PpRRgYMhyx63535946 = 99366823;    double PpRRgYMhyx30228797 = -505553983;    double PpRRgYMhyx44366108 = -806290454;    double PpRRgYMhyx16648633 = -192446705;    double PpRRgYMhyx58380322 = 65426221;    double PpRRgYMhyx84383152 = -898948898;    double PpRRgYMhyx70174528 = -692806330;    double PpRRgYMhyx73359789 = -502058409;    double PpRRgYMhyx39511794 = 30813722;    double PpRRgYMhyx98879569 = -661412806;    double PpRRgYMhyx21989420 = -266188710;    double PpRRgYMhyx18223301 = -658114798;    double PpRRgYMhyx68640815 = -722457361;    double PpRRgYMhyx98181883 = -490393759;    double PpRRgYMhyx82052800 = -725073646;    double PpRRgYMhyx53113912 = -77035520;    double PpRRgYMhyx95799484 = -647779150;    double PpRRgYMhyx43840030 = -92184071;    double PpRRgYMhyx36339117 = -798890396;    double PpRRgYMhyx60594004 = -123479483;    double PpRRgYMhyx38584477 = -925218367;    double PpRRgYMhyx54637268 = -239688484;    double PpRRgYMhyx45099751 = -508369873;    double PpRRgYMhyx94573775 = -111158401;    double PpRRgYMhyx58222156 = -304407299;    double PpRRgYMhyx6521023 = -562680897;    double PpRRgYMhyx51565372 = -622803595;    double PpRRgYMhyx95824057 = -548161952;    double PpRRgYMhyx79613464 = 61202556;    double PpRRgYMhyx83123680 = -820738200;    double PpRRgYMhyx56825521 = -104073014;    double PpRRgYMhyx30227827 = -52592391;    double PpRRgYMhyx72042220 = -944434135;    double PpRRgYMhyx33935426 = -490587998;    double PpRRgYMhyx13138390 = -866281821;    double PpRRgYMhyx1973073 = -98920073;    double PpRRgYMhyx81574981 = -655678436;    double PpRRgYMhyx59967689 = -241918966;    double PpRRgYMhyx77311572 = -842032582;    double PpRRgYMhyx36341969 = -681891462;    double PpRRgYMhyx59323974 = -621119655;    double PpRRgYMhyx25441447 = -563345710;    double PpRRgYMhyx12448709 = -83133354;    double PpRRgYMhyx91051178 = -348458039;    double PpRRgYMhyx70012593 = -587174459;    double PpRRgYMhyx46123519 = -56600903;    double PpRRgYMhyx79655027 = -927933499;    double PpRRgYMhyx2653453 = -392652904;    double PpRRgYMhyx53493663 = -994951363;    double PpRRgYMhyx68395860 = -784198412;    double PpRRgYMhyx45018259 = -417512319;    double PpRRgYMhyx94455155 = -459169584;    double PpRRgYMhyx45264565 = -584443056;    double PpRRgYMhyx94980368 = -237552084;    double PpRRgYMhyx43923684 = -214020626;    double PpRRgYMhyx8898678 = -660944694;    double PpRRgYMhyx85129045 = -997184111;    double PpRRgYMhyx49792332 = -595132053;    double PpRRgYMhyx58426477 = -888039406;    double PpRRgYMhyx51859300 = -371892882;    double PpRRgYMhyx32817781 = -176145303;    double PpRRgYMhyx74350471 = -44644379;    double PpRRgYMhyx93746324 = -463260965;    double PpRRgYMhyx56388113 = -148448079;    double PpRRgYMhyx42054049 = -457339792;    double PpRRgYMhyx91761592 = -113596319;    double PpRRgYMhyx46181081 = -713680663;    double PpRRgYMhyx34705390 = -131869364;    double PpRRgYMhyx85043493 = -624111938;    double PpRRgYMhyx80079727 = -526153573;    double PpRRgYMhyx71538931 = -421357084;    double PpRRgYMhyx35831795 = -305860184;    double PpRRgYMhyx66528458 = -250151490;    double PpRRgYMhyx99997147 = -16998934;    double PpRRgYMhyx1270031 = -502359828;    double PpRRgYMhyx13143031 = -535925842;     PpRRgYMhyx11831510 = PpRRgYMhyx27616593;     PpRRgYMhyx27616593 = PpRRgYMhyx16443588;     PpRRgYMhyx16443588 = PpRRgYMhyx76674335;     PpRRgYMhyx76674335 = PpRRgYMhyx32201870;     PpRRgYMhyx32201870 = PpRRgYMhyx25941089;     PpRRgYMhyx25941089 = PpRRgYMhyx76616321;     PpRRgYMhyx76616321 = PpRRgYMhyx35691895;     PpRRgYMhyx35691895 = PpRRgYMhyx20725122;     PpRRgYMhyx20725122 = PpRRgYMhyx29498502;     PpRRgYMhyx29498502 = PpRRgYMhyx98801235;     PpRRgYMhyx98801235 = PpRRgYMhyx51960502;     PpRRgYMhyx51960502 = PpRRgYMhyx89930748;     PpRRgYMhyx89930748 = PpRRgYMhyx92002012;     PpRRgYMhyx92002012 = PpRRgYMhyx64346820;     PpRRgYMhyx64346820 = PpRRgYMhyx48295843;     PpRRgYMhyx48295843 = PpRRgYMhyx835336;     PpRRgYMhyx835336 = PpRRgYMhyx35546464;     PpRRgYMhyx35546464 = PpRRgYMhyx21509773;     PpRRgYMhyx21509773 = PpRRgYMhyx40817743;     PpRRgYMhyx40817743 = PpRRgYMhyx38295186;     PpRRgYMhyx38295186 = PpRRgYMhyx81603682;     PpRRgYMhyx81603682 = PpRRgYMhyx55574373;     PpRRgYMhyx55574373 = PpRRgYMhyx82508161;     PpRRgYMhyx82508161 = PpRRgYMhyx63535946;     PpRRgYMhyx63535946 = PpRRgYMhyx30228797;     PpRRgYMhyx30228797 = PpRRgYMhyx44366108;     PpRRgYMhyx44366108 = PpRRgYMhyx16648633;     PpRRgYMhyx16648633 = PpRRgYMhyx58380322;     PpRRgYMhyx58380322 = PpRRgYMhyx84383152;     PpRRgYMhyx84383152 = PpRRgYMhyx70174528;     PpRRgYMhyx70174528 = PpRRgYMhyx73359789;     PpRRgYMhyx73359789 = PpRRgYMhyx39511794;     PpRRgYMhyx39511794 = PpRRgYMhyx98879569;     PpRRgYMhyx98879569 = PpRRgYMhyx21989420;     PpRRgYMhyx21989420 = PpRRgYMhyx18223301;     PpRRgYMhyx18223301 = PpRRgYMhyx68640815;     PpRRgYMhyx68640815 = PpRRgYMhyx98181883;     PpRRgYMhyx98181883 = PpRRgYMhyx82052800;     PpRRgYMhyx82052800 = PpRRgYMhyx53113912;     PpRRgYMhyx53113912 = PpRRgYMhyx95799484;     PpRRgYMhyx95799484 = PpRRgYMhyx43840030;     PpRRgYMhyx43840030 = PpRRgYMhyx36339117;     PpRRgYMhyx36339117 = PpRRgYMhyx60594004;     PpRRgYMhyx60594004 = PpRRgYMhyx38584477;     PpRRgYMhyx38584477 = PpRRgYMhyx54637268;     PpRRgYMhyx54637268 = PpRRgYMhyx45099751;     PpRRgYMhyx45099751 = PpRRgYMhyx94573775;     PpRRgYMhyx94573775 = PpRRgYMhyx58222156;     PpRRgYMhyx58222156 = PpRRgYMhyx6521023;     PpRRgYMhyx6521023 = PpRRgYMhyx51565372;     PpRRgYMhyx51565372 = PpRRgYMhyx95824057;     PpRRgYMhyx95824057 = PpRRgYMhyx79613464;     PpRRgYMhyx79613464 = PpRRgYMhyx83123680;     PpRRgYMhyx83123680 = PpRRgYMhyx56825521;     PpRRgYMhyx56825521 = PpRRgYMhyx30227827;     PpRRgYMhyx30227827 = PpRRgYMhyx72042220;     PpRRgYMhyx72042220 = PpRRgYMhyx33935426;     PpRRgYMhyx33935426 = PpRRgYMhyx13138390;     PpRRgYMhyx13138390 = PpRRgYMhyx1973073;     PpRRgYMhyx1973073 = PpRRgYMhyx81574981;     PpRRgYMhyx81574981 = PpRRgYMhyx59967689;     PpRRgYMhyx59967689 = PpRRgYMhyx77311572;     PpRRgYMhyx77311572 = PpRRgYMhyx36341969;     PpRRgYMhyx36341969 = PpRRgYMhyx59323974;     PpRRgYMhyx59323974 = PpRRgYMhyx25441447;     PpRRgYMhyx25441447 = PpRRgYMhyx12448709;     PpRRgYMhyx12448709 = PpRRgYMhyx91051178;     PpRRgYMhyx91051178 = PpRRgYMhyx70012593;     PpRRgYMhyx70012593 = PpRRgYMhyx46123519;     PpRRgYMhyx46123519 = PpRRgYMhyx79655027;     PpRRgYMhyx79655027 = PpRRgYMhyx2653453;     PpRRgYMhyx2653453 = PpRRgYMhyx53493663;     PpRRgYMhyx53493663 = PpRRgYMhyx68395860;     PpRRgYMhyx68395860 = PpRRgYMhyx45018259;     PpRRgYMhyx45018259 = PpRRgYMhyx94455155;     PpRRgYMhyx94455155 = PpRRgYMhyx45264565;     PpRRgYMhyx45264565 = PpRRgYMhyx94980368;     PpRRgYMhyx94980368 = PpRRgYMhyx43923684;     PpRRgYMhyx43923684 = PpRRgYMhyx8898678;     PpRRgYMhyx8898678 = PpRRgYMhyx85129045;     PpRRgYMhyx85129045 = PpRRgYMhyx49792332;     PpRRgYMhyx49792332 = PpRRgYMhyx58426477;     PpRRgYMhyx58426477 = PpRRgYMhyx51859300;     PpRRgYMhyx51859300 = PpRRgYMhyx32817781;     PpRRgYMhyx32817781 = PpRRgYMhyx74350471;     PpRRgYMhyx74350471 = PpRRgYMhyx93746324;     PpRRgYMhyx93746324 = PpRRgYMhyx56388113;     PpRRgYMhyx56388113 = PpRRgYMhyx42054049;     PpRRgYMhyx42054049 = PpRRgYMhyx91761592;     PpRRgYMhyx91761592 = PpRRgYMhyx46181081;     PpRRgYMhyx46181081 = PpRRgYMhyx34705390;     PpRRgYMhyx34705390 = PpRRgYMhyx85043493;     PpRRgYMhyx85043493 = PpRRgYMhyx80079727;     PpRRgYMhyx80079727 = PpRRgYMhyx71538931;     PpRRgYMhyx71538931 = PpRRgYMhyx35831795;     PpRRgYMhyx35831795 = PpRRgYMhyx66528458;     PpRRgYMhyx66528458 = PpRRgYMhyx99997147;     PpRRgYMhyx99997147 = PpRRgYMhyx1270031;     PpRRgYMhyx1270031 = PpRRgYMhyx13143031;     PpRRgYMhyx13143031 = PpRRgYMhyx11831510;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kRlNTqASQU1779452() {     double aWwXIOZFCH11825038 = -408449983;    double aWwXIOZFCH66188909 = -448060827;    double aWwXIOZFCH17756800 = -686955603;    double aWwXIOZFCH61754195 = -687455828;    double aWwXIOZFCH41041972 = -72829377;    double aWwXIOZFCH60746521 = -161401702;    double aWwXIOZFCH45163658 = -238902137;    double aWwXIOZFCH4905945 = -580783677;    double aWwXIOZFCH3465286 = -294486429;    double aWwXIOZFCH26138016 = 49267144;    double aWwXIOZFCH19121841 = -24876190;    double aWwXIOZFCH66252179 = -987797994;    double aWwXIOZFCH21783662 = -31529318;    double aWwXIOZFCH45846233 = -198516219;    double aWwXIOZFCH9518929 = -777428552;    double aWwXIOZFCH22860390 = -983186511;    double aWwXIOZFCH56688095 = -802661232;    double aWwXIOZFCH58546997 = -17096630;    double aWwXIOZFCH41938043 = -479216198;    double aWwXIOZFCH12891506 = -677315919;    double aWwXIOZFCH17964611 = -361820029;    double aWwXIOZFCH59899948 = -893325335;    double aWwXIOZFCH92452769 = -919989409;    double aWwXIOZFCH26158270 = -20159515;    double aWwXIOZFCH91978635 = -327327030;    double aWwXIOZFCH46802874 = -413759911;    double aWwXIOZFCH14587690 = -7090167;    double aWwXIOZFCH52052157 = -801413693;    double aWwXIOZFCH71058769 = -580644093;    double aWwXIOZFCH60473023 = -574380271;    double aWwXIOZFCH3587207 = 20100436;    double aWwXIOZFCH58083646 = -893880170;    double aWwXIOZFCH24964217 = -171130494;    double aWwXIOZFCH99162406 = -538094538;    double aWwXIOZFCH4347458 = -525502634;    double aWwXIOZFCH61051490 = -737880544;    double aWwXIOZFCH80582183 = -822110740;    double aWwXIOZFCH68099562 = -485656438;    double aWwXIOZFCH17366397 = -337755062;    double aWwXIOZFCH24986805 = -786467206;    double aWwXIOZFCH83655152 = -735723048;    double aWwXIOZFCH29537691 = -285976154;    double aWwXIOZFCH96622476 = -258020650;    double aWwXIOZFCH19825903 = -780443927;    double aWwXIOZFCH22602530 = -298477769;    double aWwXIOZFCH95065236 = -51733155;    double aWwXIOZFCH72797544 = -989650495;    double aWwXIOZFCH69957357 = -719919758;    double aWwXIOZFCH76880545 = -968171161;    double aWwXIOZFCH26566446 = -149705784;    double aWwXIOZFCH64824741 = -201358602;    double aWwXIOZFCH10371315 = -930307700;    double aWwXIOZFCH55061812 = -930851452;    double aWwXIOZFCH93373951 = -252658051;    double aWwXIOZFCH83814841 = -960972291;    double aWwXIOZFCH51925090 = -515124649;    double aWwXIOZFCH73736140 = -528071418;    double aWwXIOZFCH91598529 = -566796089;    double aWwXIOZFCH69775560 = -260128798;    double aWwXIOZFCH94239098 = -659069467;    double aWwXIOZFCH46158832 = -54311535;    double aWwXIOZFCH93111500 = -437488445;    double aWwXIOZFCH33847176 = 99860416;    double aWwXIOZFCH42992263 = -720106159;    double aWwXIOZFCH22550809 = -970833292;    double aWwXIOZFCH61038195 = -130996020;    double aWwXIOZFCH41287963 = -716667500;    double aWwXIOZFCH22621256 = -493434781;    double aWwXIOZFCH41498775 = -673013586;    double aWwXIOZFCH48467438 = 60451992;    double aWwXIOZFCH42278207 = -61075771;    double aWwXIOZFCH88588532 = -217004794;    double aWwXIOZFCH41180600 = -679341568;    double aWwXIOZFCH16951238 = -692748992;    double aWwXIOZFCH29236354 = -941592871;    double aWwXIOZFCH88426919 = 24156125;    double aWwXIOZFCH63277471 = -535304685;    double aWwXIOZFCH72626867 = -39545483;    double aWwXIOZFCH3555741 = -721681747;    double aWwXIOZFCH96913398 = -175593876;    double aWwXIOZFCH74005330 = -424109417;    double aWwXIOZFCH44630333 = -287170410;    double aWwXIOZFCH75171612 = -833242533;    double aWwXIOZFCH44492324 = -330938310;    double aWwXIOZFCH95648281 = -273021670;    double aWwXIOZFCH93215892 = -49591865;    double aWwXIOZFCH3021834 = -963028718;    double aWwXIOZFCH31590266 = -918472444;    double aWwXIOZFCH15347566 = -577122247;    double aWwXIOZFCH52422368 = 89622014;    double aWwXIOZFCH87315350 = -109809126;    double aWwXIOZFCH88983653 = -155314651;    double aWwXIOZFCH98324002 = -125527640;    double aWwXIOZFCH23127299 = -678685596;    double aWwXIOZFCH78827973 = -632155671;    double aWwXIOZFCH90543651 = -198234604;    double aWwXIOZFCH95690515 = -285836570;    double aWwXIOZFCH53630214 = -537914492;    double aWwXIOZFCH97275093 = -809610635;    double aWwXIOZFCH61564335 = -408449983;     aWwXIOZFCH11825038 = aWwXIOZFCH66188909;     aWwXIOZFCH66188909 = aWwXIOZFCH17756800;     aWwXIOZFCH17756800 = aWwXIOZFCH61754195;     aWwXIOZFCH61754195 = aWwXIOZFCH41041972;     aWwXIOZFCH41041972 = aWwXIOZFCH60746521;     aWwXIOZFCH60746521 = aWwXIOZFCH45163658;     aWwXIOZFCH45163658 = aWwXIOZFCH4905945;     aWwXIOZFCH4905945 = aWwXIOZFCH3465286;     aWwXIOZFCH3465286 = aWwXIOZFCH26138016;     aWwXIOZFCH26138016 = aWwXIOZFCH19121841;     aWwXIOZFCH19121841 = aWwXIOZFCH66252179;     aWwXIOZFCH66252179 = aWwXIOZFCH21783662;     aWwXIOZFCH21783662 = aWwXIOZFCH45846233;     aWwXIOZFCH45846233 = aWwXIOZFCH9518929;     aWwXIOZFCH9518929 = aWwXIOZFCH22860390;     aWwXIOZFCH22860390 = aWwXIOZFCH56688095;     aWwXIOZFCH56688095 = aWwXIOZFCH58546997;     aWwXIOZFCH58546997 = aWwXIOZFCH41938043;     aWwXIOZFCH41938043 = aWwXIOZFCH12891506;     aWwXIOZFCH12891506 = aWwXIOZFCH17964611;     aWwXIOZFCH17964611 = aWwXIOZFCH59899948;     aWwXIOZFCH59899948 = aWwXIOZFCH92452769;     aWwXIOZFCH92452769 = aWwXIOZFCH26158270;     aWwXIOZFCH26158270 = aWwXIOZFCH91978635;     aWwXIOZFCH91978635 = aWwXIOZFCH46802874;     aWwXIOZFCH46802874 = aWwXIOZFCH14587690;     aWwXIOZFCH14587690 = aWwXIOZFCH52052157;     aWwXIOZFCH52052157 = aWwXIOZFCH71058769;     aWwXIOZFCH71058769 = aWwXIOZFCH60473023;     aWwXIOZFCH60473023 = aWwXIOZFCH3587207;     aWwXIOZFCH3587207 = aWwXIOZFCH58083646;     aWwXIOZFCH58083646 = aWwXIOZFCH24964217;     aWwXIOZFCH24964217 = aWwXIOZFCH99162406;     aWwXIOZFCH99162406 = aWwXIOZFCH4347458;     aWwXIOZFCH4347458 = aWwXIOZFCH61051490;     aWwXIOZFCH61051490 = aWwXIOZFCH80582183;     aWwXIOZFCH80582183 = aWwXIOZFCH68099562;     aWwXIOZFCH68099562 = aWwXIOZFCH17366397;     aWwXIOZFCH17366397 = aWwXIOZFCH24986805;     aWwXIOZFCH24986805 = aWwXIOZFCH83655152;     aWwXIOZFCH83655152 = aWwXIOZFCH29537691;     aWwXIOZFCH29537691 = aWwXIOZFCH96622476;     aWwXIOZFCH96622476 = aWwXIOZFCH19825903;     aWwXIOZFCH19825903 = aWwXIOZFCH22602530;     aWwXIOZFCH22602530 = aWwXIOZFCH95065236;     aWwXIOZFCH95065236 = aWwXIOZFCH72797544;     aWwXIOZFCH72797544 = aWwXIOZFCH69957357;     aWwXIOZFCH69957357 = aWwXIOZFCH76880545;     aWwXIOZFCH76880545 = aWwXIOZFCH26566446;     aWwXIOZFCH26566446 = aWwXIOZFCH64824741;     aWwXIOZFCH64824741 = aWwXIOZFCH10371315;     aWwXIOZFCH10371315 = aWwXIOZFCH55061812;     aWwXIOZFCH55061812 = aWwXIOZFCH93373951;     aWwXIOZFCH93373951 = aWwXIOZFCH83814841;     aWwXIOZFCH83814841 = aWwXIOZFCH51925090;     aWwXIOZFCH51925090 = aWwXIOZFCH73736140;     aWwXIOZFCH73736140 = aWwXIOZFCH91598529;     aWwXIOZFCH91598529 = aWwXIOZFCH69775560;     aWwXIOZFCH69775560 = aWwXIOZFCH94239098;     aWwXIOZFCH94239098 = aWwXIOZFCH46158832;     aWwXIOZFCH46158832 = aWwXIOZFCH93111500;     aWwXIOZFCH93111500 = aWwXIOZFCH33847176;     aWwXIOZFCH33847176 = aWwXIOZFCH42992263;     aWwXIOZFCH42992263 = aWwXIOZFCH22550809;     aWwXIOZFCH22550809 = aWwXIOZFCH61038195;     aWwXIOZFCH61038195 = aWwXIOZFCH41287963;     aWwXIOZFCH41287963 = aWwXIOZFCH22621256;     aWwXIOZFCH22621256 = aWwXIOZFCH41498775;     aWwXIOZFCH41498775 = aWwXIOZFCH48467438;     aWwXIOZFCH48467438 = aWwXIOZFCH42278207;     aWwXIOZFCH42278207 = aWwXIOZFCH88588532;     aWwXIOZFCH88588532 = aWwXIOZFCH41180600;     aWwXIOZFCH41180600 = aWwXIOZFCH16951238;     aWwXIOZFCH16951238 = aWwXIOZFCH29236354;     aWwXIOZFCH29236354 = aWwXIOZFCH88426919;     aWwXIOZFCH88426919 = aWwXIOZFCH63277471;     aWwXIOZFCH63277471 = aWwXIOZFCH72626867;     aWwXIOZFCH72626867 = aWwXIOZFCH3555741;     aWwXIOZFCH3555741 = aWwXIOZFCH96913398;     aWwXIOZFCH96913398 = aWwXIOZFCH74005330;     aWwXIOZFCH74005330 = aWwXIOZFCH44630333;     aWwXIOZFCH44630333 = aWwXIOZFCH75171612;     aWwXIOZFCH75171612 = aWwXIOZFCH44492324;     aWwXIOZFCH44492324 = aWwXIOZFCH95648281;     aWwXIOZFCH95648281 = aWwXIOZFCH93215892;     aWwXIOZFCH93215892 = aWwXIOZFCH3021834;     aWwXIOZFCH3021834 = aWwXIOZFCH31590266;     aWwXIOZFCH31590266 = aWwXIOZFCH15347566;     aWwXIOZFCH15347566 = aWwXIOZFCH52422368;     aWwXIOZFCH52422368 = aWwXIOZFCH87315350;     aWwXIOZFCH87315350 = aWwXIOZFCH88983653;     aWwXIOZFCH88983653 = aWwXIOZFCH98324002;     aWwXIOZFCH98324002 = aWwXIOZFCH23127299;     aWwXIOZFCH23127299 = aWwXIOZFCH78827973;     aWwXIOZFCH78827973 = aWwXIOZFCH90543651;     aWwXIOZFCH90543651 = aWwXIOZFCH95690515;     aWwXIOZFCH95690515 = aWwXIOZFCH53630214;     aWwXIOZFCH53630214 = aWwXIOZFCH97275093;     aWwXIOZFCH97275093 = aWwXIOZFCH61564335;     aWwXIOZFCH61564335 = aWwXIOZFCH11825038;}
// Junk Finished
