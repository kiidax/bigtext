/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"
#include "sample.h"

namespace boar
{
    namespace fs = boost::filesystem;

    static int SampleUsage()
    {
        std::wcout << "Usage: boar sample [-f] [-s] INPUTFILE... - RATE OUTPUTFILE [RATE OUTPUTFILE]..." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -c         no simple mode" << std::endl;
        std::wcout << " -f         force overwrite output files" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " -q         quick mode (NOT IMPLEMENTED)" << std::endl;
        std::wcout << " -s         fhuffle output files (NOT IMPLEMENTED)" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << " -          seperator between input and output files" << std::endl;
        std::wcout << " RATE       sampling rate. Probability, percent or target number of lines" << std::endl;
        std::wcout << " OURPUTFILE output file" << std::endl;
        std::wcout << std::endl;

        return 1;
    }

    bool ParseSampleRate(const std::wstring &s, double &rate, uintmax_t &numberOfLines)
    {
        try
        {
            if (s.empty())
            {
                return false;
            }
            else if (s.find('.') == std::wstring::npos)
            {
                // This must be percent or number
                size_t idx = 0;
                unsigned long long v = std::stoull(s, &idx);
                if (idx == 0)
                {
                    return false;
                }
                if (idx == s.size() - 1 && s[idx] == '%')
                {
                    if (v <= 0 || v > 100)
                    {
                        return false;
                    }
                    rate = v / 100.0;
                    numberOfLines = 0;
                    return true;
                }
                else if (idx == s.size())
                {
                    if (v <= 0)
                    {
                        return false;
                    }
                    rate = 0.0;
                    numberOfLines = v;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                size_t idx;
                double v = std::stod(s, &idx);
                if (idx != s.size())
                {
                    return false;
                }
                if (v <= 0 || v > 1.0)
                {
                    return false;
                }
                rate = v;
                numberOfLines = 0;
                return true;
            }
        }
        catch (std::invalid_argument)
        {
            return false;
        }
        catch (std::out_of_range)
        {
            return false;
        }
    }

    int SampleCommand(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool noSimpleMode = false;
        bool forceOverwrite = false;
        bool shuffleOutput = false;
        bool quickMode = false;
        std::vector<fs::path> inputFileNameList;
        std::vector<OutputSpec> outputSpecList;

        if (argc <= 1)
        {
            return SampleUsage();
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            if (*p == '-')
            {
                ++p;
                if (*p == '\0')
                {
                    std::cerr << "Separator `-' is not allowed before input files." << std::endl;
                    return 1;
                }
                else
                {
                    while (*p != '\0')
                    {
                        switch (*p)
                        {
                        case 'c':
                            noSimpleMode = true;
                            break;
                        case 'f':
                            forceOverwrite = true;
                            break;
                        case 'h':
                            return SampleUsage();
                        case 'q':
                            quickMode = true;
                            break;
                        case 's':
                            shuffleOutput = true;
                            break;
                        default:
                            std::wcerr << "Unknown option `" << *p << "'." << std::endl;
                            return 1;
                        }
                        ++p;
                    }
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
            if (*p == '-')
            {
                ++p;
                if (*p == '\0')
                {
                    // Separator
                    break;
                }
                else
                {
                    std::cerr << "Options are not allowed between input files." << std::endl;
                    return 1;
                }
            }
            else
            {
                inputFileNameList.push_back(p);
            }
        }

        if (inputFileNameList.size() == 0)
        {
            std::cerr << "No input files." << std::endl;
            return 1;
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            double rate;
            uintmax_t targetNumLines;
            if (!boar::ParseSampleRate(p, rate, targetNumLines))
            {
                std::wcerr << "Invalid rate `" << p << "'." << std::endl;
                return 1;
            }
            if (optind >= argc)
            {
                std::wcerr << "Output file name is expected";
                return 1;
            }
            if (targetNumLines > 0)
            {
                outputSpecList.emplace_back(argv[optind++], targetNumLines);
            }
            else
            {
                outputSpecList.emplace_back(argv[optind++], rate);
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

        if (shuffleOutput)
        {
            std::wcerr << "Output shuffling is not supported yet." << std::endl;
            return 1;
        }

        for (auto& spec : outputSpecList)
        {
            if (spec.numberOfLines > 0)
            {
                std::wcerr << "Target number of lines is not supported yet." << std::endl;
                return 1;
            }
        }

        for (auto &fileName : inputFileNameList)
        {
            std::wcout << "Input file: " << fileName << std::endl;
        }
        for (auto &spec : outputSpecList)
        {
            if (spec.numberOfLines == 0)
            {
                std::wcout << "Output file: " << spec.fileName << " at " << 100.0 * spec.rate << "%" << std::endl;
            }
            else
            {
                std::wcout << "Output file: " << spec.fileName << " for " << spec.numberOfLines << " lines" << std::endl;
            }
        }

        if (!CheckInputFiles(inputFileNameList))
        {
            return 1;
        }

        if (!forceOverwrite)
        {
            std::vector<fs::path> outputFileNameList;
            for (auto& spec : outputSpecList) outputFileNameList.push_back(spec.fileName);
            if (!CheckOutputFiles(outputFileNameList))
            {
                return 1;
            }
        }
            
        std::srand(static_cast<int>(std::time(nullptr)));

        if (!noSimpleMode && outputSpecList.size() == 1 && outputSpecList[0].numberOfLines == 0)
        {
            std::wcout << "Only one output without target number of lines. Using simple mode." << std::endl;
            LineSampleProcessor<char> proc(inputFileNameList, outputSpecList[0].rate, outputSpecList[0].fileName);
            boar::DumpProfile([&proc]() {
                proc.Run();
                return false;
            });
        }
        else
        {
            BufferReader<LineSampleProcessor2<char>> proc;
            boar::DumpProfile([&inputFileNameList, &outputSpecList, &proc]() {
                proc.Run(inputFileNameList, outputSpecList, false);
                return false;
            });
            return 0;
        }

        return 0;
    }
}
