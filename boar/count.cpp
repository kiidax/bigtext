/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "count.h"

namespace boar
{
    namespace fs = boost::filesystem;

    static int CountUsage()
    {
        std::wcout << "Usage: boar count [OPTION]... INPUTFILE..." << std::endl;
        std::wcout << "Estimate number of lines in the file by reading only the first 100MB." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -c         full count mode" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << std::endl;
        return 1;
    }

    int CountCommand(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool fullCountMode = false;
        std::vector<fs::path> inputFileNameList;

        if (argc <= 1)
        {
            return CountUsage();
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
                        return CountUsage();
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

        for (auto &fileName : inputFileNameList)
        {
            if (fullCountMode)
            {
                boost::timer::cpu_timer timer;
                // 1059203072      404601
                // 36,762,348,544 bytes.
                // AMD E2-7110
                uintmax_t lineCount = FileCountLines<char>(fileName);
                std::cerr << timer.format() << std::endl;
                std::wcout << fileName.native() << "\tLineCount\t" << lineCount << std::endl;
            }
            else
            {
                GuessLineInfo info = FileStatLines<char>(fileName);
                std::wcout << fileName.native() << "\tMinLineSize\t" << info.minLineSize << std::endl;
                std::wcout << fileName.native() << "\tMaxLineSize\t" << info.maxLineSize << std::endl;
                std::wcout << fileName.native() << "\tAvgLineSize\t" << std::fixed << std::setprecision(2) << info.avgLineSize << std::endl;
                std::wcout << fileName.native() << "\tStdLineSize\t" << info.stdLineSize << std::endl;
                std::wcout << fileName.native() << "\tUsedLineCount\t" << info.lineCount << std::endl;
                uintmax_t size = fs::file_size(fileName);
                std::wcout << fileName.native() << "\tFileSize\t" << size << std::endl;
                if (info.isAccurate)
                {
                    std::wcout << fileName.native() << "\tEstLineCount\t" << info.lineCount << std::endl;
                }
                else
                {
                    double estLineCount = info.stdLineSize == 0 ? 1.0 : size / info.avgLineSize;
                    std::wcout << fileName.native() << "\tEstLineCount\t" << estLineCount << std::endl;
                }

                status = 0;
            }
        }

        return status;
    }
}
