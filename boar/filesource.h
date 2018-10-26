/* Boar - Boar is a collection of tools to process large text files.
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

    template <typename CharT>
    void FileLineSourceDefault(const fs::path &fileName, std::function<void(const CharT *, size_t)> callback)
    {
        uintmax_t lineCount = 0;
        std::basic_string<CharT> _previousPartialLine;

        FileSourceDefault(fileName, [&lineCount, &_previousPartialLine, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previousPartialLine.size() > 0)
                {
                    callback(_previousPartialLine.data(), _previousPartialLine.size());
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
                        if (IsNewLine(*p++))
                        {
                            _previousPartialLine.append(first, p);
                            callback(_previousPartialLine.data(), _previousPartialLine.size());
                            c++;
                            _previousPartialLine.clear();
                            break;
                        }
                    }
                }
                const CharT* lineStart = p;
                while (p != last)
                {
                    if (IsNewLine(*p++))
                    {
                        callback(lineStart, p - lineStart);
                        c++;
                        lineStart = p;
                    }
                }
                _previousPartialLine.append(lineStart, last);
                lineCount += c;
            }
        });
    }

    template <typename CharT>
    void FileWordSourceDefault(const fs::path &fileName, std::function<void(const CharT *, size_t)> callback)
    {
        uintmax_t lineCount = 0;
        std::basic_string<CharT> _previousPartialLine;

        FileSourceDefault(fileName, [&lineCount, &_previousPartialLine, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previousPartialLine.size() > 0)
                {
                    callback(_previousPartialLine.data(), _previousPartialLine.size());
                    lineCount++;
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
                        if (IsWhiteSpace(*p++))
                        {
                            _previousPartialLine.append(first, p - 1);
                            callback(_previousPartialLine.data(), _previousPartialLine.size());
                            c++;
                            _previousPartialLine.clear();
                            break;
                        }
                    }
                }
                const CharT* lineStart = p;
                while (p != last)
                {
                    if (IsWhiteSpace(*p++))
                    {
                        if (p - lineStart - 1 > 0)
                        {
                            callback(lineStart, p - lineStart - 1);
                            c++;
                        }
                        lineStart = p;
                    }
                }
                _previousPartialLine.append(lineStart, last);
                lineCount += c;
            }
        });
    }

    template <typename CharT, CharT LINE_SEPARATOR = '\n', CharT COLUMN_SEPARATOR = '\t'>
    void FileWordSourceWithColumnDefault(const fs::path &fileName, std::function<void(const CharT *, size_t, int column)> callback)
    {
        uintmax_t lineCount = 0;
        std::basic_string<CharT> _previousPartialLine;

        FileSourceDefault(fileName, [&lineCount, &_previousPartialLine, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            int column = 0;
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previousPartialLine.size() > 0)
                {
                    callback(_previousPartialLine.data(), _previousPartialLine.size(), column);
                    lineCount++;
                }
            }
            else
            {
                size_t c = 0;
                const CharT *last = reinterpret_cast<const CharT *>(s + len);
                const CharT *p = reinterpret_cast<const CharT *>(s);
                const CharT* lineStart = p;

                while (p != last)
                {
                    if (IsWhiteSpace(*p))
                    {
                        if (_previousPartialLine.size() > 0)
                        {
                            _previousPartialLine.append(lineStart, p);
                            callback(_previousPartialLine.data(), _previousPartialLine.size(), column);
                            _previousPartialLine.clear();
                        }
                        else if (p - lineStart > 0)
                        {
                            callback(lineStart, p - lineStart, column);
                        }

                        if (*p == LINE_SEPARATOR)
                        {
                            column = 0;
                            c++;
                        }
                        else if (*p == COLUMN_SEPARATOR)
                        {
                            column++;
                        }

                        lineStart = p + 1;
                    }

                    p++;
                }
                _previousPartialLine.append(lineStart, last);
                lineCount += c;
            }
        });
    }
}
