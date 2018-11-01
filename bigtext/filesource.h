/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace bigtext
{
    namespace fs = boost::filesystem;

    using data_source_callback = std::function<void(const char *, size_t)>;

    void file_source_with_memory_mapping(const fs::path &file_name, data_source_callback callback);
    void file_source_with_file_read(const fs::path &file_name, data_source_callback callback);
    void file_source_with_overlap_read(const fs::path &file_name, data_source_callback callback, uintmax_t max_size = 0);
    void file_source_default(const fs::path &file_name, data_source_callback callback, uintmax_t max_size = 0);

    template <typename CharT>
    void file_line_source_default(const fs::path &file_name, std::function<void(const CharT *, size_t)> callback)
    {
        uintmax_t line_count = 0;
        std::basic_string<CharT> _previous_partial_line;

        file_source_default(file_name, [&line_count, &_previous_partial_line, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previous_partial_line.size() > 0)
                {
                    callback(_previous_partial_line.data(), _previous_partial_line.size());
                }
            }
            else
            {
                size_t c = 0;
                const CharT *first = reinterpret_cast<const CharT *>(s);
                const CharT *last = s + len;
                const CharT *p = first;
                if (_previous_partial_line.size() > 0)
                {
                    while (p != last)
                    {
                        if (is_new_line(*p++))
                        {
                            _previous_partial_line.append(first, p);
                            callback(_previous_partial_line.data(), _previous_partial_line.size());
                            c++;
                            _previous_partial_line.clear();
                            break;
                        }
                    }
                }
                const CharT* line_start = p;
                while (p != last)
                {
                    if (is_new_line(*p++))
                    {
                        callback(line_start, p - line_start);
                        c++;
                        line_start = p;
                    }
                }
                _previous_partial_line.append(line_start, last);
                line_count += c;
            }
        });
    }

    template <typename CharT>
    void file_word_source_default(const fs::path &file_name, std::function<void(const CharT *, size_t)> callback)
    {
        uintmax_t line_count = 0;
        std::basic_string<CharT> _previous_partial_line;

        file_source_default(file_name, [&line_count, &_previous_partial_line, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previous_partial_line.size() > 0)
                {
                    callback(_previous_partial_line.data(), _previous_partial_line.size());
                    line_count++;
                }
            }
            else
            {
                size_t c = 0;
                const CharT *first = reinterpret_cast<const CharT *>(s);
                const CharT *last = s + len;
                const CharT *p = first;
                if (_previous_partial_line.size() > 0)
                {
                    while (p != last)
                    {
                        if (is_white_space(*p++))
                        {
                            _previous_partial_line.append(first, p - 1);
                            callback(_previous_partial_line.data(), _previous_partial_line.size());
                            c++;
                            _previous_partial_line.clear();
                            break;
                        }
                    }
                }
                const CharT* line_start = p;
                while (p != last)
                {
                    if (is_white_space(*p++))
                    {
                        if (p - line_start - 1 > 0)
                        {
                            callback(line_start, p - line_start - 1);
                            c++;
                        }
                        line_start = p;
                    }
                }
                _previous_partial_line.append(line_start, last);
                line_count += c;
            }
        });
    }

    template <typename CharT, CharT LINE_SEPARATOR = '\n', CharT COLUMN_SEPARATOR = '\t'>
    void file_word_source_with_column_default(const fs::path &file_name, std::function<void(const CharT *, size_t, int column)> callback)
    {
        uintmax_t line_count = 0;
        std::basic_string<CharT> _previous_partial_line;

        file_source_default(file_name, [&line_count, &_previous_partial_line, callback](const char *_s, size_t _len)
        {
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            int column = 0;
            size_t len = _len / sizeof(CharT);

            if (s == nullptr)
            {
                if (_previous_partial_line.size() > 0)
                {
                    callback(_previous_partial_line.data(), _previous_partial_line.size(), column);
                    line_count++;
                }
            }
            else
            {
                size_t c = 0;
                const CharT *last = reinterpret_cast<const CharT *>(s + len);
                const CharT *p = reinterpret_cast<const CharT *>(s);
                const CharT* line_start = p;

                while (p != last)
                {
                    if (is_white_space(*p))
                    {
                        if (_previous_partial_line.size() > 0)
                        {
                            _previous_partial_line.append(line_start, p);
                            callback(_previous_partial_line.data(), _previous_partial_line.size(), column);
                            _previous_partial_line.clear();
                        }
                        else if (p - line_start > 0)
                        {
                            callback(line_start, p - line_start, column);
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

                        line_start = p + 1;
                    }

                    p++;
                }
                _previous_partial_line.append(line_start, last);
                line_count += c;
            }
        });
    }
}
