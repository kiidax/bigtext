/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

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

    bool CheckInputFiles(const std::vector<fs::path> &inputFileNameList)
    {
        for (auto& fileName : inputFileNameList)
        {
            if (!fs::is_regular_file(fileName))
            {
                std::wcerr << "`" << fileName.wstring() << "' doesn't exist." << std::endl;
                return false;
            }
        }
        return true;
    }

    bool CheckOutputFiles(const std::vector<fs::path>& outputFileNameList)
    {
        for (auto& fileName : outputFileNameList)
        {
            if (fs::exists(fileName))
            {
                std::wcerr << "`" << fileName.wstring() << "' already exists." << std::endl;
                return false;
            }
        }
        return true;
    }
}
