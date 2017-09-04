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

        virtual void ProcessBuffer(const void *data_, size_t n_)
        {
            const char* data = reinterpret_cast<const charT*>(data_);
            const size_t n = n_ / sizeof(charT);
            size_t c = 0;
            for (size_t i = 0; i < n; i++)
            {
                if (data[i] == '\n') c++;
            }
            _lineCount += c;
        }

        virtual void EndContent()
        {
            std::wcout << _currentFilePath << '\t' << _lineCount << std::endl;
        }
    };
}
