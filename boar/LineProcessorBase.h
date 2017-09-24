/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    template <typename charT>
    class LineProcessorBase : public ProcessorBase
    {
    protected:
        std::vector<charT> _previousPartialLine;
        size_t _lineCount;

    public:
        LineProcessorBase() {}
        virtual ~LineProcessorBase() {}

        virtual void ProcessBuffer(const void* first_, const void* last_)
        {
            const charT* first = reinterpret_cast<const charT*>(first_);
            const charT* last = reinterpret_cast<const charT*>(last_);
            size_t c = 0;
            const charT* lineStart = first;
            for (const charT* p = first; p != last; p++)
            {
                if (*p == '\n')
                {
                    ProcessLine(lineStart, p);
                    lineStart = p + 1;
                    c++;
                }
            }
            _lineCount += c;
        }

        virtual bool ProcessLine(const charT* first, const charT* last) = 0;
    };
}
