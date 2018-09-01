/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "filesource.h"

namespace boar
{
    namespace fs = boost::filesystem;

    static const uintmax_t GUESS_LINE_SIZE = 100 * 1024 * 1024;

    template<typename CharT>
    uintmax_t FileCountLines(const fs::path &fname)
    {
        uintmax_t lineCount = 0;
        CharT lastChar = '\n';
        FileSourceDefault(fname, [&lineCount, &lastChar](const char *_s, size_t _len) {
            const CharT *s = reinterpret_cast<const CharT*>(_s);
            size_t len = _len / sizeof(CharT);
            if (s == nullptr)
            {
                if (lastChar != '\n') lineCount++;
            }
            else
            {
                uintmax_t c = 0;
                for (size_t i = 0; i < len; i++)
                {
                    if (s[i] == '\n') c++;
                }
                lineCount += c;
                if (len > 0) lastChar = s[len - 1];
            }
        });
        return lineCount;
    }

    struct GuessLineInfo
    {
        uintmax_t minLineSize;
        uintmax_t maxLineSize;
        double avgLineSize;
        double stdLineSize;
        uintmax_t lineCount;
        bool isAccurate;
    };

    template<typename CharT>
    GuessLineInfo FileStatLines(const fs::path &fname)
    {
        uintmax_t curSize = 0;
        uintmax_t totalLineSize = 0;
        uintmax_t totalSqLineSize = 0;
        GuessLineInfo info = { MAXUINT, 0, 0.0, 0.0, 0, false };
        FileSourceDefault(fname, [&info, &curSize, &totalLineSize, &totalSqLineSize](const char *_s, size_t _len) {
            const CharT *s = reinterpret_cast<const CharT*>(_s);
            size_t len = _len / sizeof(CharT);
            uintmax_t c = info.lineCount;
            uintmax_t l = curSize;
            if (s == nullptr)
            {
                assert(len == 0);
                if (l > 0)
                {
                    if (info.minLineSize > l) info.minLineSize = l;
                    if (info.maxLineSize < l) info.maxLineSize = l;
                    c++;
                    totalLineSize += l;
                    totalSqLineSize += l * l;
                    l = 0;
                }
                info.isAccurate = true;
            }
            else
            {
                for (size_t i = 0; i < len; i++)
                {
                    l++;
                    if (s[i] == '\n')
                    {
                        if (info.minLineSize > l) info.minLineSize = l;
                        if (info.maxLineSize < l) info.maxLineSize = l;
                        c++;
                        totalLineSize += l;
                        totalSqLineSize += l * l;
                        l = 0;
                    }
                }
            }
            info.lineCount = c;
            curSize = l;
        }, GUESS_LINE_SIZE);
        if (info.lineCount == 0)
        {
            info.minLineSize = 0;
            info.avgLineSize = static_cast<double>(curSize);
            info.stdLineSize = 0.0;
        }
        else if (info.lineCount == 1)
        {
            info.avgLineSize = static_cast<double>(totalLineSize);
            info.stdLineSize = 0.0;
        }
        else
        {
            info.avgLineSize = static_cast<double>(totalLineSize) / info.lineCount;
            double x = static_cast<double>(totalSqLineSize * info.lineCount - totalLineSize * totalLineSize);
            double y = static_cast<double>(info.lineCount * (info.lineCount - 1));
            info.stdLineSize = std::sqrt(x / y);
        }
        return info;
    }
}
