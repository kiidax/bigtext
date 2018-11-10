/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "bigtext.h"
#include "vocab.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static int vocab_usage()
    {
        std::wcout << "Usage: bigtext vocab [OPTION]... INPUTFILE... [[-o|-m COLUMN] OUTPUTFILE]..." << std::endl;
        std::wcout << "Count words in the files and make vocabulary list." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -f         force overwrite output files" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << " -o         count words in all columns" << std::endl;
        std::wcout << " -c COLUMN  count words in COLUMN-th column" << std::endl;
        std::wcout << " OUTPUTFILE output file" << std::endl;
        return 0;
    }

    int vocab_command(int argc, wchar_t *argv[])
    {
        int optind = 1;
        bool no_simple_mode = false;
        bool force_overwrite = false;
        bool shuffle_output = false;
        bool has_output_all = false;
        std::vector<fs::path> input_file_name_list;
        std::vector<vocab_output_spec> output_spec_list;

        if (argc <= 1)
        {
            return vocab_usage();
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
                    force_overwrite = true;
                    break;
                case 'h':
                    return vocab_usage();
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

            input_file_name_list.push_back(p);
        }

        if (input_file_name_list.size() == 0)
        {
            std::cerr << "No input files." << std::endl;
            return 1;
        }

        while (optind < argc)
        {
            const wchar_t *p = argv[optind++];
            bool next_is_number = false;
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

            if (has_output_all)
            {
                std::wcerr << "Another output after -o option is not allowed." << std::endl;
                return 1;
            }

            switch (*p)
            {
            case 'c':
                next_is_number = true;
                break;
            case 'o':
                has_output_all = true;
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
                    if (next_is_number)
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

            uintmax_t column_number = 0;

            if (next_is_number)
            {
                if (!try_parse_number(p, column_number) || column_number > INT_MAX)
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

            std::wcout << p << "\tTargetColumn\t" << column_number << std::endl;
            if (column_number > 0)
            {
                output_spec_list.emplace_back(p, static_cast<int>(column_number - 1));
            }
            else
            {
                output_spec_list.emplace_back(p);
            }
        }

        if (output_spec_list.size() == 0)
        {
            std::wcerr << "No output files." << std::endl;
            return 1;
        }

        // Verify all the input files exist
        if (!check_input_files(input_file_name_list))
        {
            return 1;
        }

        if (!force_overwrite)
        {
            // Verify none of the output files exists
            std::vector<fs::path> output_file_name_list;
            for (auto &spec : output_spec_list) output_file_name_list.push_back(spec.file_name);
            if (!check_output_files(output_file_name_list))
            {
                return 1;
            }
        }

        int status;

        boost::timer::cpu_timer timer;

        if (output_spec_list.size() == 1)
        {
            if (output_spec_list[0].column == -1)
            {
                // Count all columns.
                auto &file_name = output_spec_list[0].file_name;
                file_count_vocab<char>(input_file_name_list, file_name);
                status = 0;
            }
            else
            {
                // Count one column.
                file_count_vocab<char>(input_file_name_list, output_spec_list[0]);
                status = 0;
            }
        }
        else
        {
            // Count specified columns.
            file_count_vocab<char>(input_file_name_list, output_spec_list);
            status = 0;
        }

        std::cerr << timer.format() << std::endl;

        return status;
    }
}