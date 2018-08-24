/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"
#include "LineSampleProcessor.h"

namespace boar
{
    static int MainUsage()
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
        if (argc == 1)
        {
            return MainUsage();
        }
        else if (argc >= 2)
        {
            const std::wstring commandName(argv[1]);
            if (commandName == L"count")
            {
                return CountCommand(argc - 1, argv + 1);
            }
            else if (commandName == L"sample")
            {
                return SampleCommand(argc - 1, argv + 1);
            }
            else
            {
                std::wcerr << L"Unknown command `" << commandName << L"'." << std::endl;
                exit(1);
            }
        }
        else
        {
            return MainUsage();
        }
        return 0;
    }
}
