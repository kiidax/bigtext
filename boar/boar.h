/* Boar - Boar is a collection of tools to process text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

namespace boar {

    namespace fs = boost::filesystem;

    int Main(int argc, wchar_t *argv[]);
    int DumpProfile(std::function<bool()> func);
    int CountCommand(int argc, wchar_t *argv[]);
    int SampleCommand(int argc, wchar_t *argv[]);
    bool CheckInputFiles(const std::vector<fs::path> &inputFileNameList);
    bool CheckOutputFiles(const std::vector<fs::path> &outputFileNameList);
    bool ParseRate(const std::wstring &s, double &rate, uintmax_t &numberOfLines);

    inline std::u16string ToUnicode(const char *s)
    {
        return std::u16string(s, s + strlen(s));
    }

    inline std::string FromUnicode(const char16_t* s)
    {
        std::u16string t(s);
        return std::string(t.begin(), t.end());
    }
}