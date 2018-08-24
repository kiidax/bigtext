/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "Processor.h"

namespace boar
{
    template <typename charT>
    class LineCountProcessor :
        public Processor
    {
    private:
        size_t _lineCount;

    public:
        LineCountProcessor() {}
        ~LineCountProcessor() {}

        virtual void BeginFile()
        {
            _lineCount = 0;
        }

        virtual void ProcessBlock(_In_ const char* s, _In_ size_t len)
        {
            const charT* first = reinterpret_cast<const charT*>(s);
            const charT* last = reinterpret_cast<const charT*>(s + len);
            size_t c = 0;
            for (const charT *p = first; p != last; ++p)
            {
                if (*p == '\n') c++;
            }
            _lineCount += c;
        }

        virtual void EndFile()
        {
            std::wcout << _currentFilePath << '\t' << _lineCount << std::endl;
        }
    };
}
