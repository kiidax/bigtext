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
        int column; // 0 indexed. -1 for all.

        VocabOutputSpec(const fs::path &fileName) : VocabOutputSpec(fileName, -1) {}
        VocabOutputSpec(const fs::path &fileName, int column) : fileName(fileName), column(column) {}
    };

    template <typename CharT, typename StringT = std::basic_string<CharT>>
    bool WriteVocabCount(const std::unordered_map<StringT, uintmax_t> &vocabCount, const fs::path &outputFileName)
    {
        using StringCountT = std::pair<StringT, uintmax_t>;
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
            return false;
        }
        for (auto &kv : sortedKeyValue)
        {
            out << kv.first << '\t' << kv.second << std::endl;
        }
        return true;
    }

    template <typename CharT, typename StringT = std::basic_string<CharT>>
    void IncrementVocabCount(std::unordered_map<StringT, uintmax_t> &vocabCount, const CharT *s, size_t len)
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

    template <typename CharT>
    void FileCountVocab(const std::vector<fs::path> &inputFileNameList, const fs::path& outputFileName)
    {
        using StringT = std::basic_string<CharT>;
        std::unordered_map<StringT, uintmax_t> vocabCount;
        for (auto& fileName : inputFileNameList)
        {
            FileWordSourceDefault<CharT>(fileName, [&vocabCount](const CharT *s, size_t len) {
                if (s != nullptr)
                {
                    IncrementVocabCount(vocabCount, s, len);
                }
                return true;
            });
        }

        WriteVocabCount<CharT>(vocabCount, outputFileName);
    }

    template <typename CharT>
    void FileCountVocab(const std::vector<fs::path> &inputFileNameList, const VocabOutputSpec &outputSpec)
    {
        using StringT = std::basic_string<CharT>;
        std::unordered_map<StringT, uintmax_t> vocabCount;
        int targetColumn = outputSpec.column;

        for (auto& fileName : inputFileNameList)
        {
            FileWordSourceWithColumnDefault<CharT>(fileName, [&vocabCount, targetColumn](const CharT *s, size_t len, int column) {
                if (s != nullptr)
                {
                    if (column == targetColumn)
                    {
                        IncrementVocabCount(vocabCount, s, len);
                    }
                }
                return true;
            });
        }

        WriteVocabCount<CharT>(vocabCount, outputSpec.fileName);
    }

    template <typename CharT>
    void FileCountVocab(const std::vector<fs::path> &inputFileNameList, const std::vector<VocabOutputSpec> &outputSpecList)
    {
        using StringT = std::basic_string<CharT>;
        using StringCountT = std::pair<StringT, uintmax_t>;
        std::vector<std::auto_ptr<std::unordered_map<StringT, uintmax_t>>> vocabCountList;
        for (auto &outputSpec : outputSpecList)
        {
            while (vocabCountList.size() <= outputSpec.column)
            {
                vocabCountList.emplace_back();
            }
            vocabCountList[outputSpec.column].reset(new std::unordered_map<StringT, uintmax_t >());
        }

        for (auto& fileName : inputFileNameList)
        {
            FileWordSourceWithColumnDefault<CharT>(fileName, [&vocabCountList](const CharT *s, size_t len, int column) {
                if (s != nullptr)
                {
                    if (column < vocabCountList.size())
                    {
                        auto vocabCount = vocabCountList[column].get();
                        if (vocabCount != nullptr)
                        {
                            IncrementVocabCount(*vocabCount, s, len);
                        }
                    }
                }
                return true;
            });
        }

        for (auto &outputSpec : outputSpecList)
        {
            auto vocabCount = vocabCountList[outputSpec.column];
            WriteVocabCount<CharT>(*vocabCount, outputSpec.fileName);
        }
    }
}