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

        virtual void ProcessBlock(_In_ const void* first_, _In_ const void* last_)
        {
            const charT* first = reinterpret_cast<const charT*>(first_);
            const charT* last = reinterpret_cast<const charT*>(last_);
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
