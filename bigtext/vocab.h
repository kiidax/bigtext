/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "filesource.h"

namespace bigtext
{
    namespace fs = boost::filesystem;

    static const uintmax_t QUICK_LINE_COUNT = 100 * 1024 * 1024;

    struct vocab_output_spec
    {
        fs::path file_name;
        int column; // 0 indexed. -1 for all.

        vocab_output_spec(const fs::path &file_name) : vocab_output_spec(file_name, -1) {}
        vocab_output_spec(const fs::path &file_name, int column) : file_name(file_name), column(column) {}
    };

    template <typename CharT, typename StringT = std::basic_string<CharT>>
    bool write_vocab_count(const std::unordered_map<StringT, uintmax_t> &vocab_count, const fs::path &output_file_name)
    {
        using StringCountT = std::pair<StringT, uintmax_t>;
        std::vector<StringCountT> sorted_key_value(vocab_count.cbegin(), vocab_count.cend());
        std::sort(sorted_key_value.begin(), sorted_key_value.end(), [](const StringCountT &x, const StringCountT &y)
        {
            return x.second == y.second ? x.first < y.first : x.second > y.second;
        });

        fs::basic_ofstream<CharT> out;
        out.open(output_file_name, std::ios::out);
        if (!out.is_open())
        {
            std::wcerr << __wcserror(output_file_name.native().c_str());
            return false;
        }
        for (auto &kv : sorted_key_value)
        {
            out << kv.first << '\t' << kv.second << std::endl;
        }
        return true;
    }

    template <typename CharT, typename StringT = std::basic_string<CharT>>
    void increment_vocab_count(std::unordered_map<StringT, uintmax_t> &vocab_count, const CharT *s, size_t len)
    {
        StringT key(s, s + len);
        auto it = vocab_count.find(key);
        if (it != vocab_count.end())
        {
            (*it).second += 1;
        }
        else
        {
            vocab_count.emplace(std::move(key), 1);
        }
    }

    template <typename CharT>
    void file_count_vocab(const std::vector<fs::path> &input_file_name_list, const fs::path& output_file_name)
    {
        using StringT = std::basic_string<CharT>;
        std::unordered_map<StringT, uintmax_t> vocab_count;
        for (auto &file_name : input_file_name_list)
        {
            file_word_source_default<CharT>(file_name, [&vocab_count](const CharT *s, size_t len) {
                if (s != nullptr)
                {
                    increment_vocab_count(vocab_count, s, len);
                }
                return true;
            });
        }

        write_vocab_count<CharT>(vocab_count, output_file_name);
    }

    template <typename CharT>
    void file_count_vocab(const std::vector<fs::path> &input_file_name_list, const vocab_output_spec &output_spec)
    {
        using StringT = std::basic_string<CharT>;
        std::unordered_map<StringT, uintmax_t> vocab_count;
        int target_column = output_spec.column;

        for (auto &file_name : input_file_name_list)
        {
            file_word_source_with_column_default<CharT>(file_name, [&vocab_count, target_column](const CharT *s, size_t len, int column) {
                if (s != nullptr)
                {
                    if (column == target_column)
                    {
                        increment_vocab_count(vocab_count, s, len);
                    }
                }
                return true;
            });
        }

        write_vocab_count<CharT>(vocab_count, output_spec.file_name);
    }

    template <typename CharT>
    void file_count_vocab(const std::vector<fs::path> &input_file_name_list, const std::vector<vocab_output_spec> &output_spec_list)
    {
        using StringT = std::basic_string<CharT>;
        using StringCountT = std::pair<StringT, uintmax_t>;
        std::vector<std::auto_ptr<std::unordered_map<StringT, uintmax_t>>> vocab_count_list;
        for (auto &output_spec : output_spec_list)
        {
            while (vocab_count_list.size() <= output_spec.column)
            {
                vocab_count_list.emplace_back();
            }
            vocab_count_list[output_spec.column].reset(new std::unordered_map<StringT, uintmax_t >());
        }

        for (auto &file_name : input_file_name_list)
        {
            file_word_source_with_column_default<CharT>(file_name, [&vocab_count_list](const CharT *s, size_t len, int column) {
                if (s != nullptr)
                {
                    if (column < vocab_count_list.size())
                    {
                        auto vocab_count = vocab_count_list[column].get();
                        if (vocab_count != nullptr)
                        {
                            increment_vocab_count(*vocab_count, s, len);
                        }
                    }
                }
                return true;
            });
        }

        for (auto &output_spec : output_spec_list)
        {
            auto vocab_count = vocab_count_list[output_spec.column];
            write_vocab_count<CharT>(*vocab_count, output_spec.file_name);
        }
    }
}