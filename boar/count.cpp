/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"

namespace boar
{
    namespace fs = boost::filesystem;

    int DumpProfile(std::function<bool()> func)
    {
        clock_t startTime = clock();
        int code = func();
        clock_t endTime = clock();
        clock_t t = endTime - startTime;
        std::wcout << "Success" << '\t' << code << std::endl;
        std::wcout << "TimeMs" << '\t' << t << std::endl;
        return code;
    }

    static int CountUsage()
    {
        std::wcout << "usage: boar couunt inputfiles" << std::endl;
        return 1;
    }

    template<typename CharT>
    uintmax_t FileCountLines(fs::path &fname)
    {
        uintmax_t lineCount = 0;
        FileSourceWithOverlapRead(fname, [&lineCount](const char *s, size_t len) {
            const CharT *p = reinterpret_cast<const CharT*>(s);
            uintmax_t c = 0;
            for (size_t i = 0; i < len; i++)
            {
                if (p[i] == '\n') c++;
            }
            lineCount += c;
        });
        return lineCount;
    }

    int CountCommand(int argc, wchar_t *argv[])
    {
        int status = DumpProfile([argc, &argv]()
        {
            // 1059203072      404601
            // 36,762,348,544 bytes.
            // AMD E2-7110
            for (int i = 1; i < argc; i++)
            {
                fs::path fname(argv[i]);
                uintmax_t lineCount = FileCountLines<char>(fname);
                std::wcout << fname << '\t' << lineCount << std::endl;
            }
            return true;
        });
        return status;
    }
}
