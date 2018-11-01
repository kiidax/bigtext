/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "filesource.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static const uintmax_t GUESS_LINE_SIZE = 100 * 1024 * 1024;

    template<typename CharT>
    uintmax_t file_count_lines(const fs::path &fname)
    {
        uintmax_t line_count = 0;
        CharT last_char = '\n';
        file_source_default(fname, [&line_count, &last_char](const char *_s, size_t _len) {
            const CharT *s = reinterpret_cast<const CharT*>(_s);
            size_t len = _len / sizeof(CharT);
            if (s == nullptr)
            {
                if (last_char != '\n') line_count++;
            }
            else
            {
                uintmax_t c = 0;
                for (size_t i = 0; i < len; i++)
                {
                    if (s[i] == '\n') c++;
                }
                line_count += c;
                if (len > 0) last_char = s[len - 1];
            }
        });
        return line_count;
    }

    struct guess_line_info
    {
        uintmax_t min_line_size;
        uintmax_t max_line_size;
        double avg_line_size;
        double std_line_size;
        uintmax_t line_count;
        bool is_accurate;
    };

    template<typename CharT>
    guess_line_info file_stat_lines(const fs::path &fname)
    {
        uintmax_t cur_size = 0;
        uintmax_t total_line_size = 0;
        uintmax_t total_sq_line_size = 0;
        guess_line_info info = { MAXUINT, 0, 0.0, 0.0, 0, false };
        file_source_default(fname, [&info, &cur_size, &total_line_size, &total_sq_line_size](const char *_s, size_t _len) {
            const CharT *s = reinterpret_cast<const CharT*>(_s);
            size_t len = _len / sizeof(CharT);
            uintmax_t c = info.line_count;
            uintmax_t l = cur_size;
            if (s == nullptr)
            {
                assert(len == 0);
                if (l > 0)
                {
                    if (info.min_line_size > l) info.min_line_size = l;
                    if (info.max_line_size < l) info.max_line_size = l;
                    c++;
                    total_line_size += l;
                    total_sq_line_size += l * l;
                    l = 0;
                }
                info.is_accurate = true;
            }
            else
            {
                for (size_t i = 0; i < len; i++)
                {
                    l++;
                    if (s[i] == '\n')
                    {
                        if (info.min_line_size > l) info.min_line_size = l;
                        if (info.max_line_size < l) info.max_line_size = l;
                        c++;
                        total_line_size += l;
                        total_sq_line_size += l * l;
                        l = 0;
                    }
                }
            }
            info.line_count = c;
            cur_size = l;
        }, GUESS_LINE_SIZE);
        if (info.line_count == 0)
        {
            info.min_line_size = 0;
            info.avg_line_size = static_cast<double>(cur_size);
            info.std_line_size = 0.0;
        }
        else if (info.line_count == 1)
        {
            info.avg_line_size = static_cast<double>(total_line_size);
            info.std_line_size = 0.0;
        }
        else
        {
            info.avg_line_size = static_cast<double>(total_line_size) / info.line_count;
            double x = static_cast<double>(total_sq_line_size * info.line_count - total_line_size * total_line_size);
            double y = static_cast<double>(info.line_count * (info.line_count - 1));
            info.std_line_size = std::sqrt(x / y);
        }
        return info;
    }
}
