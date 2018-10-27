/* bigtext - bigtext is a collection of tools to process large text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

namespace bigtext {

    namespace fs = boost::filesystem;

    extern const wchar_t* PROGRAM_NAME;
    extern const int MAJOR_VERSION;
    extern const int MINOR_VERSION;
    extern const int BUILD_VERSION;
    extern const int REVISION_VERSION;

    int Main(int argc, wchar_t *argv[]);
    int CountCommand(int argc, wchar_t *argv[]);
    int SampleCommand(int argc, wchar_t *argv[]);
    int VocabCommand(int argc, wchar_t *argv[]);
    int VersionCommand(int argc, wchar_t *argv[]);
    std::wstring GetVersionString();
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

#ifdef WIN32
    template <typename T>
    class heap_vector
    {
    public:
        heap_vector(size_t max_size)
        {
            ptr_ = VirtualAlloc(NULL, max_size, MEM_COMMIT, PAGE_READWRITE);
            if (ptr_ == NULL)
            {
                throw std::bad_alloc();
            }
        }

        ~heap_vector()
        {
            VirtualFree(ptr_, 0, MEM_RELEASE);
        }

        T *ptr() const { return reinterpret_cast<T *>(ptr_); }

        LPVOID ptr_;
    };
#endif
}