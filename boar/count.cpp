/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"
#include "LineCountProcessor.h"
#include "LineSampleProcessor.h"

namespace boar
{
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

    int CountCommand(int argc, wchar_t *argv[])
    {
        const std::vector<std::wstring> args2(argv + 2, argv + argc);
        int status = DumpProfile([&args2]()
        {
            // 1059203072      404601
            // 36,762,348,544 bytes.
            // AMD E2-7110
            std::auto_ptr<Processor> proc(new LineCountProcessor<char>());
            proc->ProcessFileList(args2);
            return true;
        });
        return status;
    }
}
