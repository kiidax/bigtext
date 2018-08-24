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
    namespace po = boost::program_options;

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
            "   sample     Sample lines from file.\n"
            << std::endl;
        return 1;
    }

    int Main(int argc, wchar_t *argv[])
    {
        int status;
        if (argc == 1)
        {
            return Usage();
        }
        else if (argc >= 2)
        {
            const std::wstring commandName(argv[1]);
            if (commandName == L"sample")
            {
                return sample_command(argc - 1, argv + 1);
            }
            else if (commandName == L"count")
            {
                const std::vector<std::wstring> args2(argv + 2, argv + argc);
                for (int i = 0; i < 2; i++)
                {
                    status = DumpProfile([&args2, i]()
                    {
                        // 1059203072      404601
                        // 36,762,348,544 bytes.
                        // AMD E2-7110
                        std::auto_ptr<Processor> proc(new LineCountProcessor<char>());
                        proc->hoge = i;
                        proc->ProcessFileList(args2);
                        return true;
                    });
                }
            }
            else
            {
                std::wcerr << L"Unknown command `" << commandName << L"'." << std::endl;
                exit(1);
            }
        }
        else
        {
            return Usage();
        }
        return 0;
    }
}
