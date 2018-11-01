/* bigtext - bigtext is a collection of tools to process large text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "bigtext.h"

namespace bigtext
{
    static int main_usage()
    {
        std::wcout <<
            L"usage: bigtext COMMAND [ARGS]\n"
            "\n"
            "bigtext is a collection of tools to process large text files.\n"
            "\n"
            "List of commands:\n"
            "\n"
            "   count      Count the number of lines in the files.\n"
            "   sample     Sample lines from the files.\n"
            "   vocab      Count the words in the files.\n"
            "   version    Show the version info.\n";
        return 0;
    }

    const wchar_t *PROGRAM_NAME = L"bigtext";
    const int MAJOR_VERSION = 0;
    const int MINOR_VERSION = 9;
    const int BUILD_VERSION = 1;
    const int REVISION_VERSION = 0;

    std::wstring get_version_string()
    {
        return std::to_wstring(MAJOR_VERSION) + L"."
            + std::to_wstring(MINOR_VERSION) + L"."
            + std::to_wstring(BUILD_VERSION) + L"."
            + std::to_wstring(REVISION_VERSION);
    }

    static int version_command(int argc, wchar_t *argv[])
    {
        std::wcout
            << PROGRAM_NAME << " " << get_version_string() << std::endl
            << L"Copyright (C) 2017 Katsuya Iida. All rights reserved." << std::endl;
        return 0;
    }

    int main(int argc, wchar_t *argv[])
    {
        if (argc == 1)
        {
            return main_usage();
        }
        else if (argc >= 2)
        {
            const std::wstring command_name(argv[1]);
            if (command_name == L"count")
            {
                return count_command(argc - 1, argv + 1);
            }
            else if (command_name == L"sample")
            {
                return sample_command(argc - 1, argv + 1);
            }
            else if (command_name == L"vocab")
            {
                return vocab_command(argc - 1, argv + 1);
            }
            else if (command_name == L"version")
            {
                return version_command(argc - 1, argv + 1);
            }
            else
            {
                std::wcerr << L"Unknown command `" << command_name << L"'." << std::endl;
                exit(1);
            }
        }
        else
        {
            return main_usage();
        }
        return 0;
    }

    bool check_input_files(const std::vector<fs::path> &input_file_name_list)
    {
        for (auto &file_name : input_file_name_list)
        {
            if (!fs::is_regular_file(file_name))
            {
                std::wcerr << "`" << file_name.wstring() << "' doesn't exist." << std::endl;
                return false;
            }
        }
        return true;
    }

    bool check_output_files(const std::vector<fs::path>& output_file_name_list)
    {
        for (auto &it = output_file_name_list.cbegin(); it != output_file_name_list.cend(); ++it)
        {
            auto &file_name = *it;
            if (fs::exists(file_name))
            {
                std::wcerr << "`" << file_name.native() << "' already exists." << std::endl;
                return false;
            }

            for (auto &it2 = output_file_name_list.cbegin(); it2 != it; ++it2)
            {
                auto &file_name2 = *it2;
                if (file_name == file_name2)
                {
                    std::wcerr << "Duplicated output file `" << file_name.native() << "'." << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

    bool try_parse_rate(const std::wstring &s, double &rate)
    {
        try
        {
            if (s.empty())
            {
                return false;
            }

            size_t idx;
            double v = std::stod(s, &idx);
            if (idx == s.size() - 1 && s[idx] == '%')
            {
                if (v <= 0 || v > 100)
                {
                    return false;
                }
                rate = v / 100.0;
                return true;
            }
            else if (idx != s.size())
            {
                return false;
            }
            if (v <= 0 || v > 1.0)
            {
                return false;
            }
            rate = v;
            return true;
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

    bool try_parse_number(const std::wstring &s, uintmax_t &number_of_lines)
    {
        try
        {
            if (s.empty())
            {
                return false;
            }

            size_t idx = 0;
            unsigned long long v = std::stoull(s, &idx);
            if (idx != s.size())
            {
                return false;
            }
            if (v <= 0)
            {
                return false;
            }
            number_of_lines = v;
            return true;
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
}
