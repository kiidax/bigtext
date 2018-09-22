/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "filesource.h"

namespace boar
{
    namespace fs = boost::filesystem;

    struct VocabOutputSpec
    {
        fs::path fileName;
        int column;

        VocabOutputSpec(const fs::path &fileName) : VocabOutputSpec(fileName, 0) {}
        VocabOutputSpec(const fs::path &fileName, int column) : fileName(fileName), column(column) {}
    };

    template <typename CharT>
    void FileCountVocab(const std::vector<fs::path> &inputFileNameList, fs::path& outputFileName)
    {
        using StringT = std::basic_string<CharT>;
        using StringCountT = std::pair<StringT, uintmax_t>;
        std::unordered_map<StringT, uintmax_t> vocabCount;
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
                        vocabCount.emplace(std::move(key), 1);
                    }
                }
                return true;
            });
        }

        std::vector<StringCountT> sortedKeyValue(vocabCount.cbegin(), vocabCount.cend());
        std::sort(sortedKeyValue.begin(), sortedKeyValue.end(), [](const StringCountT &x, const StringCountT &y)
        {
            return x.second == y.second ? x.first < y.first : x.second > y.second;
        });

        fs::basic_ofstream<CharT> out;
        out.open(outputFileName, std::ios::out);
        if (!out.is_open())
        {
            std::wcerr << __wcserror(outputFileName.native().c_str());
            return;
        }
        for (auto &kv : sortedKeyValue)
        {
            out << kv.first << '\t' << kv.second << std::endl;
        }
    }
}