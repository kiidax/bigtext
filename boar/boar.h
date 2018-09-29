/* Boar - Boar is a collection of tools to process text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

namespace boar {

    namespace fs = boost::filesystem;

    int Main(int argc, wchar_t *argv[]);
    int CountCommand(int argc, wchar_t *argv[]);
    int SampleCommand(int argc, wchar_t *argv[]);
    int VocabCommand(int argc, wchar_t *argv[]);
    bool CheckInputFiles(const std::vector<fs::path> &inputFileNameList);
    bool CheckOutputFiles(const std::vector<fs::path> &outputFileNameList);
    bool TryParseRate(const std::wstring &s, double &rate);
    bool TryParseNumber(const std::wstring &s, uintmax_t &numberOfLines);
    uintmax_t GetPhysicalMemorySize();

    template <typename CharT>
    bool IsNewLine(CharT ch)
    {
        return ch == '\n';
    }

    template <typename CharT>
    bool IsWhiteSpace(CharT ch)
    {
        return ch <= ' ';
    }
}