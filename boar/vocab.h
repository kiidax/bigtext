/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "filesource.h"

namespace boar
{
    template <typename CharT>
    class FileCountVocabProcessor
    {
    public:
        typedef CharT CharType;

    private:
        fs::basic_ofstream<CharT> _out;
        std::map<std::basic_string<CharT>, uintmax_t> _vocabCount;

    public:
        FileCountVocabProcessor()
        {
        }

        void Run(const std::vector<fs::path> &inputFileNameList, fs::path& outputFileName)
        {
            _out.open(outputFileName, std::ios::out);
            if (!_out.is_open())
            {
                std::wcerr << "cannot open" << std::endl;
                return;
            }
            for (auto& fileName : inputFileNameList)
            {
                FileWordSourceDefault(fileName, *this);
            }
            std::vector<std::pair<std::basic_string<CharT>, uintmax_t>> v(_vocabCount.begin(), _vocabCount.end());
            std::sort(v.begin(), v.end(), [](std::pair<std::basic_string<CharT>, uintmax_t> &x, std::pair<std::basic_string<CharT>, uintmax_t> &y)
            {
                return x.second > y.second;
            });
            for (auto &kv : v)
            {
                _out << kv.first << '\t' << kv.second << std::endl;
            }
        }

        bool ProcessLine(const CharT *s, size_t len)
        {
            if (len > 1)
            {
                std::basic_string<CharT> key(s, s + len - 1);
                auto it = _vocabCount.find(key);
                if (it != _vocabCount.end())
                {
                    (*it).second += 1;
                }
                else
                {
                    _vocabCount.insert(it, std::pair<std::basic_string<CharT>, uintmax_t>(std::move(key), 1));
                }
            }
            return true;
        }
    };

    template <typename CharT>
    class FileCountVocabProcessor2
    {
    public:
        typedef CharT CharType;

    private:

    public:
        FileCountVocabProcessor2()
        {
        }

        void Run(const std::vector<fs::path> &inputFileNameList, fs::path& outputFileName)
        {
            fs::basic_ofstream<CharT> _out;
            std::map<std::basic_string<CharT>, uintmax_t> _vocabCount;

            _out.open(outputFileName, std::ios::out);
            if (!_out.is_open())
            {
                std::wcerr << "cannot open" << std::endl;
                return;
            }
            for (auto& fileName : inputFileNameList)
            {
                FileWordSourceDefault<CharT>(fileName, [&_vocabCount](const CharT *s, size_t len) {
                    if (len > 1)
                    {
                        std::basic_string<CharT> key(s, s + len - 1);
                        auto it = _vocabCount.find(key);
                        if (it != _vocabCount.end())
                        {
                            (*it).second += 1;
                        }
                        else
                        {
                            _vocabCount.insert(it, std::pair<std::basic_string<CharT>, uintmax_t>(std::move(key), 1));
                        }
                    }
                    return true;
                });
            }
            std::vector<std::pair<std::basic_string<CharT>, uintmax_t>> v(_vocabCount.begin(), _vocabCount.end());
            std::sort(v.begin(), v.end(), [](std::pair<std::basic_string<CharT>, uintmax_t> &x, std::pair<std::basic_string<CharT>, uintmax_t> &y)
            {
                return x.second > y.second;
            });
            for (auto &kv : v)
            {
                _out << kv.first << '\t' << kv.second << std::endl;
            }
        }
    };
}