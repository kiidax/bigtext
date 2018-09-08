/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "shuffle.h"

namespace boar
{
    namespace fs = boost::filesystem;

    static int ShuffleUsage()
    {
        std::wcout << "Usage: boar shuffle [OPTION]... INPUTFILE... - OUTPUT" << std::endl;
        std::wcout << "Shuffle files." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -b         shuffle big files" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << std::endl;
        return 1;
    }

    int ShuffleCommand(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool interleavingMode = false;
        std::vector<fs::path> inputFileNameList;

        if (argc <= 1)
        {
            return ShuffleUsage();
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            if (*p == '-')
            {
                ++p;
                while (*p != '\0')
                {
                    switch (*p)
                    {
                    case 'b':
                        interleavingMode = true;
                        break;
                    case 'h':
                        return ShuffleUsage();
                    default:
                        std::wcerr << "Unknown option `" << *p << "'." << std::endl;
                        return 1;
                    }
                    ++p;
                }
            }
            else
            {
                // Input files start.
                optind--;
                break;
            }
        }

        while (optind < argc - 2)
        {
            const wchar_t *p = argv[optind++];
            inputFileNameList.push_back(p);
        }

        if (std::wstring(argv[optind++]) != L"-")
        {
            std::wcerr << "- expected" << std::endl;
            return 1;
        }

        fs::path outputFileName(argv[optind]);

        if (inputFileNameList.size() == 0)
        {
            std::wcerr << "No input files." << std::endl;
            return 1;
        }

        if (!CheckInputFiles(inputFileNameList))
        {
            return 1;
        }

        int status;

        for (auto &fileName : inputFileNameList)
        {
            if (interleavingMode)
            {
                std::wcerr << "Not supported yet." << std::endl;
                status = 1;
            }
            else
            {
                status = DumpProfile([&fileName, &outputFileName]()
                {
                    FileShuffleLines<char>(fileName, outputFileName);
                    return true;
                });
            }
        }

        return status;
    }
}
