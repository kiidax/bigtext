/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "ProcessorBase.h"

namespace boar
{
    template <typename charT>
    class LineCountProcessor :
        public ProcessorBase
    {
    private:
        size_t _lineCount;

    public:
        LineCountProcessor() {}
        ~LineCountProcessor() {}

        virtual void BeginContent()
        {
            _lineCount = 0;
        }

        virtual void ProcessBuffer(const void* first_, const void* last_)
        {
            const charT* first = reinterpret_cast<const charT*>(first_);
            const charT* last = reinterpret_cast<const charT*>(last_);
            size_t c = 0;
            for (const charT *it = first; it != last; ++it)
            {
                if (*it == '\n') c++;
            }
            _lineCount += c;
        }

        virtual void EndContent()
        {
            std::wcout << _currentFilePath << '\t' << _lineCount << std::endl;
        }
    };
}
