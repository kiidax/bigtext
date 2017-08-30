/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "LineProcessorBase.h"

namespace boar
{
    template <typename charT>
    class FindStringProcessor : public LineProcessorBase<charT>
    {
    private:
        size_t _lineCount;

    public:
        FindStringProcessor()
        {
        }

        bool ProcessLines(const void *addr, size_t n)
        {
            intptr_t addrValue = reinterpret_cast<intptr_t>(addr);
            const charT* first = reinterpret_cast<const charT*>(addrValue);
            const charT* last = reinterpret_cast<const charT*>(addrValue + n);
            size_t c = 0;
            const charT* lineStart = first;
            for (const charT* p = first; p != last; p++)
            {
                if (*p == '\n')
                {
                    const std::basic_string<wchar_t> s(lineStart, p);
                    std::wcout << s << std::endl;
                    lineStart = p + 1;
                    c++;
                }
            }
            _lineCount += c;
        }
    };
}