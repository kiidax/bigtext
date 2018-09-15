/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "vocab.h"

namespace boar
{
    namespace fs = boost::filesystem;

    static int VocabUsage()
    {
        std::wcout << "Usage: boar vocab [OPTION]... INPUTFILE..." << std::endl;
        std::wcout << "Estimate number of lines in the file by reading only the first 100MB." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -c         full count mode" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << std::endl;
        return 1;
    }

    int VocabCommand(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool fullCountMode = false;
        std::vector<fs::path> inputFileNameList;

        if (argc <= 1)
        {
            return VocabUsage();
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
                    case 'c':
                        fullCountMode = true;
                        break;
                    case 'h':
                        return VocabUsage();
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

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            inputFileNameList.push_back(p);
        }

        if (inputFileNameList.size() == 0)
        {
            std::cerr << "No input files." << std::endl;
            return 1;
        }

        if (!CheckInputFiles(inputFileNameList))
        {
            return 1;
        }

        int status;

        if (fullCountMode)
        {
            status = DumpProfile([&inputFileNameList]()
            {
                // 1059203072      404601
                // 36,762,348,544 bytes.
                // AMD E2-7110
                std::cout << "class" << std::endl;
                FileCountVocabProcessor<char> proc;
                proc.Run(inputFileNameList, fs::path(L"vocab.txt"));
                return true;
            });
        }
        else
        {
            status = DumpProfile([&inputFileNameList]()
            {
                // 1059203072      404601
                // 36,762,348,544 bytes.
                // AMD E2-7110
                std::cout << "func" << std::endl;
                FileCountVocabProcessor2<char> proc;
                proc.Run(inputFileNameList, fs::path(L"vocab.txt"));
                return true;
            });
            status = 0;
        }

        return status;
    }
}