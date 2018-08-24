/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "Processor.h"

namespace boar
{
    template <typename charT>
    class LineProcessor : public Processor
    {
    protected:
        std::basic_string<charT> _previousPartialLine;
        size_t _lineCount;
        std::basic_ofstream<charT> out;

    public:
        LineProcessor() {}
        virtual ~LineProcessor() {}

    protected:
        virtual void ProcessBlock(const char* s, size_t len)
        {
            const charT* first = reinterpret_cast<const charT*>(s);
            const charT* last = reinterpret_cast<const charT*>(s + len);
            size_t c = 0;
            const charT* p = first;
            if (_previousPartialLine.size() > 0)
            {
                while (p != last)
                {
                    if (*p++ == '\n')
                    {
                        _previousPartialLine.append(first, p);
                        ProcessLine(_previousPartialLine.data(), _previousPartialLine.data() + _previousPartialLine.size());
                        c++;
                        _previousPartialLine.clear();
                        break;
                    }
                }
            }
            const charT* lineStart = p;
            while (p != last)
            {
                if (*p++ == '\n')
                {
                    ProcessLine(lineStart, p);
                    c++;
                    lineStart = p;
                }
            }
            _previousPartialLine.append(lineStart, last);
            _lineCount += c;
        }

        void OutputText(const charT* first_, const charT* last_)
        {
            const char *first = reinterpret_cast<const char *>(first_);
            const char *last = reinterpret_cast<const char *>(last_);
            _outf.write(first, last - first);
        }

        virtual bool ProcessLine(const charT* first, const charT* last) = 0;
    };
}
