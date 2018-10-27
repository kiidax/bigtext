/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "bigtext.h"
#include "vocab.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static int VocabUsage()
    {
        std::wcout << "Usage: bigtext vocab [OPTION]... INPUTFILE... [[-o|-m COLUMN] OUTPUTFILE]..." << std::endl;
        std::wcout << "Count words in the files and make vocabulary list." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -q         quick mode" << std::endl;
        std::wcout << " -f         force overwrite output files" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << " -o         count words in all columns" << std::endl;
        std::wcout << " -c COLUMN  count words in COLUMN-th column" << std::endl;
        std::wcout << " OUTPUTFILE output file" << std::endl;
        return 0;
    }

    int VocabCommand(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool noSimpleMode = false;
        bool forceOverwrite = false;
        bool shuffleOutput = false;
        bool hasOutputAll = false;
        bool quickMode = false;
        std::vector<fs::path> inputFileNameList;
        std::vector<VocabOutputSpec> outputSpecList;

        if (argc <= 1)
        {
            return VocabUsage();
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            if (*p != '-')
            {
                // Input files start.
                optind--;
                break;
            }

            ++p;
            if (*p == '\0')
            {
                std::cerr << "An option is expected." << std::endl;
                return 1;
            }

            while (*p != '\0')
            {
                switch (*p)
                {
                case 'f':
                    forceOverwrite = true;
                    break;
                case 'h':
                    return VocabUsage();
                case 'q':
                    quickMode = true;
                    break;
                case 'c':
                case 'o':
                    std::wcerr << "No input files." << std::endl;
                    return 1;
                default:
                    std::wcerr << "Unknown option `" << *p << "'." << std::endl;
                    return 1;
                }
                ++p;
            }
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            if (*p == '-')
            {
                // Output files start.
                optind--;
                break;
            }

            inputFileNameList.push_back(p);
        }

        if (inputFileNameList.size() == 0)
        {
            std::cerr << "No input files." << std::endl;
            return 1;
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            bool nextIsNumber = false;
            if (*p++ != '-')
            {
                std::wcerr << "-m or -o is expected." << std::endl;
                return 1;
            }

            if (*p == '\0')
            {
                std::cerr << "An option is expected." << std::endl;
                return 1;
            }

            if (hasOutputAll)
            {
                std::wcerr << "Another output after -o option is not allowed." << std::endl;
                return 1;
            }

            switch (*p)
            {
            case 'c':
                nextIsNumber = true;
                break;
            case 'o':
                hasOutputAll = true;
                break;
            default:
                std::wcerr << "Unknown option `" << *p << "'." << std::endl;
                return 1;
            }
            p++;

            if (*p == '\0')
            {
                if (optind >= argc)
                {
                    if (nextIsNumber)
                    {
                        std::wcerr << "Line number is expected." << std::endl;
                    }
                    else
                    {
                        std::wcerr << "Output file name is expected" << std::endl;
                    }
                    return 1;
                }
                p = argv[optind++];
            }

            uintmax_t columnNumber = 0;

            if (nextIsNumber)
            {
                if (!TryParseNumber(p, columnNumber) || columnNumber > INT_MAX)
                {
                    std::wcerr << "Invalid column number `" << p << "'." << std::endl;
                    return 1;
                }

                if (optind >= argc)
                {
                    std::wcerr << "Output file name is expected" << std::endl;
                    return 1;
                }

                p = argv[optind++];
            }

            if (*p == '-')
            {
                std::wcerr << "Output file name is expected" << std::endl;
                return 1;
            }

            if (nextIsNumber)
            {
                std::wcout << p << "\tTargetColumn\t" << columnNumber << std::endl;
                outputSpecList.emplace_back(p, static_cast<int>(columnNumber - 1));
            }
            else
            {
                std::wcout << p << "\tTargetColumn\t" << 0 << std::endl;
                outputSpecList.emplace_back(p);
            }
        }

        if (outputSpecList.size() == 0)
        {
            std::wcerr << "No output files." << std::endl;
            return 1;
        }

        if (quickMode)
        {
            std::wcerr << "Quick mode is not supported yet." << std::endl;
            return 1;
        }

        // Verify all the input files exist
        if (!CheckInputFiles(inputFileNameList))
        {
            return 1;
        }

        if (!forceOverwrite)
        {
            // Verify none of the output files exists
            std::vector<fs::path> outputFileNameList;
            for (auto& spec : outputSpecList) outputFileNameList.push_back(spec.fileName);
            if (!CheckOutputFiles(outputFileNameList))
            {
                return 1;
            }
        }

        int status;

        boost::timer::cpu_timer timer;

        if (outputSpecList.size() == 1)
        {
            if (outputSpecList[0].column == -1)
            {
                // Count all columns.
                auto &fileName = outputSpecList[0].fileName;
                FileCountVocab<char>(inputFileNameList, fileName);
                status = 0;
            }
            else
            {
                // Count one column.
                FileCountVocab<char>(inputFileNameList, outputSpecList[0]);
                status = 0;
            }
        }
        else
        {
            // Count specified columns.
            FileCountVocab<char>(inputFileNameList, outputSpecList);
            status = 0;
        }

        std::cerr << timer.format() << std::endl;

        return status;
    }
}