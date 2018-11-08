/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "bigtext.h"
#include "filesource.h"
#include "count.h"
#include "sample.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static int sample_usage()
    {
        std::wcout << "Usage: bigtext sample [OPTION]... INPUTFILE... [[-o|-n LINES|-r RATE] OUTPUTFILE]..." << std::endl;
        std::wcout << std::endl;
        std::wcout << " -c N       shuffle output files with N interleaving" << std::endl;
        std::wcout << " -f         force overwrite output files" << std::endl;
        std::wcout << " -h         show this help message" << std::endl;
        std::wcout << " -q         quick mode" << std::endl;
        std::wcout << " -s         shuffle output files" << std::endl;
        std::wcout << " INPUTFILE  input file" << std::endl;
        std::wcout << " -o         sample all lines" << std::endl;
        std::wcout << " -n LINES   sample around n lines" << std::endl;
        std::wcout << " -r RATE    sampling rate. Probability (0.0,1.0] or percent (0,100]%" << std::endl;
        std::wcout << " OUTPUTFILE output file" << std::endl;
        return 0;
    }

    static bool has_number_of_lines(const std::vector<sample_output_spec> &output_spec_list)
    {
        return std::any_of(output_spec_list.cbegin(), output_spec_list.cend(), [](auto &spec) { return spec.number_of_lines != 0; });
    }

    static bool has_sample_rate(const std::vector<sample_output_spec> &output_spec_list)
    {
        return std::any_of(output_spec_list.cbegin(), output_spec_list.cend(), [](auto &spec) { return spec.number_of_lines == 0 && spec.rate < 1.0; });
    }

    static bool has_sample_all(const std::vector<sample_output_spec> &output_spec_list)
    {
        return std::any_of(output_spec_list.cbegin(), output_spec_list.cend(), [](auto &spec) { return spec.number_of_lines == 0 && spec.rate == 1.0; });
    }

    static void convert_to_rate(std::vector<sample_output_spec> &output_spec_list, double total_number_of_lines)
    {
        for (auto &spec : output_spec_list)
        {
            if (spec.number_of_lines != 0)
            {
                if (total_number_of_lines > 0.0)
                {
                    spec.rate = static_cast<double>(spec.number_of_lines) / total_number_of_lines;
                }
                else
                {
                    spec.rate = 1.0;
                }
                spec.number_of_lines = 0;
                std::wcout << spec.file_name.native() << "\tRate\t" << spec.rate << std::endl;
            }
        }
    }

    static void convert_to_number_of_lines(std::vector<sample_output_spec> &output_spec_list, double total_number_of_lines)
    {
        for (auto &spec : output_spec_list)
        {
            if (spec.rate > 0.0)
            {
                spec.number_of_lines = static_cast<uintmax_t>(spec.rate * total_number_of_lines);
                spec.rate = 0.0;
                std::wcout << spec.file_name.native() << "\tNumberOfLines\t" << spec.number_of_lines << std::endl;
            }
        }
    }

    template <typename CharT>
    static double guess_total_number_of_lines(const std::vector<fs::path> &input_file_name_list)
    {
        double total_number_of_lines = 0;

        for (auto &file_name : input_file_name_list)
        {
            guess_line_info info = file_stat_lines<CharT>(file_name);
            double est_line_count;
            if (info.is_accurate)
            {
                std::wcout << file_name.native() << "\tEstLineCount\t" << info.line_count << std::endl;
                est_line_count = static_cast<double>(info.line_count);
            }
            else
            {
                uintmax_t size = fs::file_size(file_name);
                est_line_count = info.avg_line_size == 0 ? 1.0 : (size / (sizeof(CharT) * info.avg_line_size));
                std::wcout << file_name.native() << "\tEstLineCount\t" << est_line_count << std::endl;
            }
            total_number_of_lines += est_line_count;
        }

        return total_number_of_lines;
    }


    static uintmax_t get_total_file_size(const std::vector<fs::path> &file_name_list)
    {
        uintmax_t size = 0;
        for (auto &file_name : file_name_list)
        {
            size += fs::file_size(file_name);
        }
        return size;
    }

    int sample_command(int argc, wchar_t *argv[])
    {
        int optind = 1;
        uintmax_t interleaving_size = 0;
        bool force_overwrite = false;
        bool shuffle_output = false;
        bool has_output_all = false;
        bool quick_mode = false;
        std::vector<fs::path> input_file_name_list;
        std::vector<sample_output_spec> output_spec_list;

        if (argc <= 1)
        {
            return sample_usage();
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

            bool next_is_number = false;
            while (*p != '\0')
            {
                switch (*p)
                {
                case 'c':
                    next_is_number = true;
                    break;
                case 'f':
                    force_overwrite = true;
                    break;
                case 'h':
                    return sample_usage();
                case 'q':
                    quick_mode = true;
                    break;
                case 's':
                    shuffle_output = true;
                    break;
                case 'n':
                case 'o':
                case 'r':
                    std::wcerr << "No input files." << std::endl;
                    return 1;
                default:
                    std::wcerr << "Unknown option `" << *p << "'." << std::endl;
                    return 1;
                }
                ++p;

                if (next_is_number)
                {
                    if (*p == '\0')
                    {
                        if (optind >= argc)
                        {
                            std::wcerr << "Interlaving size is expected." << std::endl;
                            return 1;
                        }
                        p = argv[optind++];
                    }

                    if (!try_parse_number(p, interleaving_size))
                    {
                        std::wcerr << "Invalid interleaving size." << std::endl;
                        return 1;
                    }
                    break;
                }
            }
        }

        if (interleaving_size > 0 && !shuffle_output)
        {
            std::wcerr << "Interleaving size is allowed only in the shuffle mode." << std::endl;
            return 1;
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
            bool next_is_rate = false;
            bool next_is_number = false;
            if (*p++ != '-')
            {
                std::wcerr << "-r, -n or -o is expected." << std::endl;
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
            case 'n':
                next_is_number = true;
                break;
            case 'o':
                has_output_all = true;
                break;
            case 'r':
                next_is_rate = true;
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
                    if (next_is_rate)
                    {
                        std::wcerr << "Rate is expected." << std::endl;
                    }
                    else if (next_is_number)
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

            double rate = 0.0;
            uintmax_t target_num_lines = 0;

            if (next_is_rate)
            {
                if (!try_parse_rate(p, rate))
                {
                    std::wcerr << "Invalid rate `" << p << "'." << std::endl;
                    return 1;
                }
            }
            else if (next_is_number)
            {
                if (!try_parse_number(p, target_num_lines))
                {
                    std::wcerr << "Invalid line number `" << p << "'." << std::endl;
                    return 1;
                }
            }

            if (next_is_rate || next_is_number)
            {
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

            if (next_is_rate)
            {
                std::wcout << p << "\tTargetRate\t" << rate << std::endl;
                output_spec_list.emplace_back(p, rate);
            }
            else if (next_is_number)
            {
                std::wcout << p << "\tTargetLineCount\t" << target_num_lines << std::endl;
                output_spec_list.emplace_back(p, target_num_lines);
            }
            else
            {
                std::wcout << p << "\tTargetRate\t" << 1.0 << std::endl;
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

        std::srand(static_cast<int>(std::time(nullptr)));

        boost::timer::cpu_timer timer;

        if (quick_mode)
        {
            if (shuffle_output)
            {
                std::wcerr << "-s is not allowed with the quick mode. The quick mode shuffles outputs." << std::endl;
                return 1;
            }

            if (input_file_name_list.size() != 1)
            {
                std::wcerr << "Only one input file is allowed." << std::endl;
                return 1;
            }

            if (has_sample_all(output_spec_list))
            {
                std::wcerr << "Sampling all lines doesn't make sense with quick mode." << std::endl;
                return 1;
            }

            if (has_sample_rate(output_spec_list))
            {
                std::cout << "Target rate is specified. Guessing number of lines." << std::endl;
                double total_number_of_lines = guess_total_number_of_lines<char>(input_file_name_list);
                convert_to_number_of_lines(output_spec_list, total_number_of_lines);
            }

            file_quick_sample_file_lines<char>(input_file_name_list[0], output_spec_list, shuffle_output);
        }
        else if (shuffle_output)
        {
            uintmax_t physical_memory_size;
            uintmax_t total_file_size;

            if (interleaving_size != 1)
            {
                physical_memory_size = get_physical_memory_size();
                if (interleaving_size == 0)
                {
                    total_file_size = get_total_file_size(input_file_name_list);
                    if (total_file_size < physical_memory_size * 8 / 10)
                    {
                        // if the files are small, then we don't try interleaving.
                        interleaving_size = 1;
                    }
                }
            }

            if (interleaving_size == 1)
            {
                file_shuffle_lines<char>(input_file_name_list, output_spec_list);
            }
            else
            {
                std::wcout << "\tMaxBufferSize\t" << physical_memory_size << std::endl;
                heap_vector<char> heap;
                heap.alloc(SHUFFLE_MIN_BUFFER_SIZE, physical_memory_size);
                size_t buffer_size = heap.size();
                if (interleaving_size == 0)
                {
                    interleaving_size = (total_file_size * 8 / 10 + buffer_size) / buffer_size;
                }
                assert(interleaving_size >= 1);
                std::wcout << "\tInterleavingSize\t" << interleaving_size << std::endl;
                std::wcout << "\tBufferSize\t" << heap.size() << std::endl;
                file_shuffle_lines<char>(input_file_name_list, output_spec_list, interleaving_size, heap.ptr(), heap.size());
            }
        }
        else
        {
            if (has_number_of_lines(output_spec_list))
            {
                std::cout << "Target number of lines is specified. Guessing number of lines." << std::endl;
                double total_number_of_lines = guess_total_number_of_lines<char>(input_file_name_list);
                convert_to_rate(output_spec_list, total_number_of_lines);
            }

            if (output_spec_list.size() == 1 && output_spec_list[0].number_of_lines == 0)
            {
                std::wcout << "Only one output without target number of lines. Using simple mode." << std::endl;
                assert(output_spec_list[0].number_of_lines == 0);
                file_line_sample<char>(input_file_name_list, output_spec_list[0].rate, output_spec_list[0].file_name);
            }
            else
            {
                file_line_sample<char>(input_file_name_list, output_spec_list);
            }
        }

        std::cerr << timer.format() << std::endl;

        return 0;
    }
}
