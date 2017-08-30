/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"
#include "LineCountProcessor.h"
#include "FindStringProcessor.h"

namespace boar
{
    bool LineCount(const std::vector<std::wstring>& args)
    {
    }

    int DumpProfile(std::function<size_t()> func)
    {
        clock_t startTime = clock();
        size_t lineCount = func();
        clock_t endTime = clock();
        clock_t t = endTime - startTime;
        std::wcout << lineCount << '\t' << t << std::endl;
        return 0;
    }

    int Usage()
    {
        std::wcout <<
            L"usage: boar <command> [<args>]\n"
            "\n"
            "Boar is a toolkit to process text files.\n"
            "\n"
            "List of commands:\n"
            "\n"
            "   find       Find a string in lines.\n"
            "   count      Count the number of lines.\n"
            << std::endl;
        return 1;
    }

    int Main(const std::vector<std::wstring>& args)
    {
        int status;
        if (args.size() == 1)
        {
            return Usage();
        }
        else if (args.size() == 2)
        {
            const std::wstring commandName(args[0]);
            const std::vector<std::wstring> args2(args.begin() + 1, args.end());
            if (commandName == L"find")
            {
                status = DumpProfile([&args2]()
                {
                    // 1059203072      404601
                    // 36,762,348,544 bytes.
                    // AMD E2-7110
                    std::auto_ptr<ProcessorBase> proc(new FindStringProcessor<char>());
                    proc->ProcessFileList(args2);
                    return true;
                });
            }
            else if (commandName == L"count")
            {
                status = DumpProfile([&args2]()
                {
                    // 1059203072      404601
                    // 36,762,348,544 bytes.
                    // AMD E2-7110
                    std::auto_ptr<ProcessorBase> proc(new LineCountProcessor<char>());
                    proc->ProcessFileList(args2);
                    return true;
                });
            }
        }
        else
        {
            return Usage();
        }
        return 0;
    }
}
