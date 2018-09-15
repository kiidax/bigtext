/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    namespace fs = boost::filesystem;

    using DataSourceCallback = std::function<void(const char *, size_t)>;

    void FileSourceWithMemoryMapping(const fs::path &fileName, DataSourceCallback callback);
    void FileSourceWithFileRead(const fs::path &fileName, DataSourceCallback callback);
    void FileSourceWithOverlapRead(const fs::path &fileName, DataSourceCallback callback, uintmax_t maxSize = 0);
    void FileSourceDefault(const fs::path &fileName, DataSourceCallback callback, uintmax_t maxSize = 0);

    template <class LineProcessorT, typename CharT = LineProcessorT::CharType>
    void FileLineSourceDefault(const fs::path &fileName, LineProcessorT& proc)
    {
        uintmax_t lineCount = 0;
        std::basic_string<CharT> _previousPartialLine;

        FileSourceDefault(fileName, [&lineCount, &_previousPartialLine, &proc](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previousPartialLine.size() > 0)
                {
                    proc.ProcessLine(_previousPartialLine.data(), _previousPartialLine.size());
                }
            }
            else
            {
                size_t c = 0;
                const CharT *first = reinterpret_cast<const CharT *>(s);
                const CharT *last = s + len;
                const CharT *p = first;
                if (_previousPartialLine.size() > 0)
                {
                    while (p != last)
                    {
                        if (*p++ == '\n')
                        {
                            _previousPartialLine.append(first, p);
                            proc.ProcessLine(_previousPartialLine.data(), _previousPartialLine.size());
                            c++;
                            _previousPartialLine.clear();
                            break;
                        }
                    }
                }
                const CharT* lineStart = p;
                while (p != last)
                {
                    if (*p++ == '\n')
                    {
                        proc.ProcessLine(lineStart, p - lineStart);
                        c++;
                        lineStart = p;
                    }
                }
                _previousPartialLine.append(lineStart, last);
                lineCount += c;
            }
        });
    }
}
