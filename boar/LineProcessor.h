/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    template <typename charT>
    class LineProcessor : public Processor
    {
    protected:
        std::basic_string<charT> _previousPartialLine;
        size_t _lineCount;

    public:
        LineProcessor() {}
        virtual ~LineProcessor() {}

        virtual void ProcessBlock(const void* first_, const void* last_)
        {
            const charT* first = reinterpret_cast<const charT*>(first_);
            const charT* last = reinterpret_cast<const charT*>(last_);
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

        virtual bool ProcessLine(const charT* first, const charT* last) = 0;
    };
}
