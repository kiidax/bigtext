/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "filesource.h"

namespace boar
{
    template <typename CharT>
    void FileCountVocab(const std::vector<fs::path> &inputFileNameList, fs::path& outputFileName)
    {
        using StringT = std::basic_string<CharT>;
        using StringCountT = std::pair<StringT, uintmax_t>;
        std::map<StringT, uintmax_t> vocabCount;
        for (auto& fileName : inputFileNameList)
        {
            FileWordSourceDefault<CharT>(fileName, [&vocabCount](const CharT *s, size_t len) {
                if (len > 0)
                {
                    StringT key(s, s + len);
                    auto it = vocabCount.find(key);
                    if (it != vocabCount.end())
                    {
                        (*it).second += 1;
                    }
                    else
                    {
                        vocabCount.insert(it, StringCountT(std::move(key), 1));
                    }
                }
                return true;
            });
        }

        std::vector<StringCountT> v(vocabCount.begin(), vocabCount.end());
        std::sort(v.begin(), v.end(), [](StringCountT &x, StringCountT &y)
        {
            return x.second > y.second;
        });

        fs::basic_ofstream<CharT> out;
        out.open(outputFileName, std::ios::out);
        if (!out.is_open())
        {
            std::wcerr << "cannot open" << std::endl;
            return;
        }
        for (auto &kv : v)
        {
            out << kv.first << '\t' << kv.second << std::endl;
        }
    }
}