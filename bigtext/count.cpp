/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "bigtext.h"
#include "count.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static int count_usage()
    {
        std::wcout << "Usage: bigtext count [OPTION]... INPUTFILE..." << std::endl;
        std::wcout << "Count number of lines in the file." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -c         full count mode" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        return 0;
    }

    template <typename CharT>
    static void dump_file_stat(const fs::path &file_name)
    {
        guess_line_info info = file_stat_lines<CharT>(file_name);
        std::wcout << file_name.native() << "\tMinLineSize\t" << info.min_line_size << std::endl;
        std::wcout << file_name.native() << "\tMaxLineSize\t" << info.max_line_size << std::endl;
        std::wcout << file_name.native() << "\tAvgLineSize\t" << std::fixed << std::setprecision(2) << info.avg_line_size << std::endl;
        std::wcout << file_name.native() << "\tStdLineSize\t" << info.std_line_size << std::endl;
        std::wcout << file_name.native() << "\tUsedLineCount\t" << info.line_count << std::endl;
        uintmax_t size = fs::file_size(file_name);
        std::wcout << file_name.native() << "\tFileSize\t" << size << std::endl;
        if (info.is_accurate)
        {
            std::wcout << file_name.native() << "\tEstLineCount\t" << info.line_count << std::endl;
        }
        else
        {
            double est_line_count = info.avg_line_size == 0 ? 1.0 : (size / sizeof(CharT) / info.avg_line_size);
            std::wcout << file_name.native() << "\tEstLineCount\t" << est_line_count << std::endl;
        }
    }

    int count_command(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool full_count_mode = false;
        std::vector<fs::path> input_file_name_list;

        if (argc <= 1)
        {
            return count_usage();
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
                        full_count_mode = true;
                        break;
                    case 'h':
                        return count_usage();
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
            input_file_name_list.push_back(p);
        }

        if (input_file_name_list.size() == 0)
        {
            std::cerr << "No input files." << std::endl;
            return 1;
        }

        if (!check_input_files(input_file_name_list))
        {
            return 1;
        }

        int status = 0;

        for (auto &file_name : input_file_name_list)
        {
            if (full_count_mode)
            {
                boost::timer::cpu_timer timer;
                // 1059203072      404601
                // 36,762,348,544 bytes.
                // AMD E2-7110
                uintmax_t line_count = file_count_lines<char>(file_name);
                std::cerr << timer.format() << std::endl;
                std::wcout << file_name.native() << "\tLineCount\t" << line_count << std::endl;
            }
            else
            {
                dump_file_stat<char>(file_name);
            }
        }

        return status;
    }
}
