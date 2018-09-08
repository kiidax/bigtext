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

    class FileWriter
    {
    protected:
        fs::ofstream _out;

    public:
        void Open(const fs::path& path)
        {
            _out.open(path, std::ios::out | std::ios::binary);
            if (!_out.is_open())
            {
                std::wcerr << "Cannot open the file." << std::endl;
            }
        }

        void Write(const char *s, size_t len)
        {
            _out.write(s, len);
        }

        void Close()
        {
            _out.close();
        }
    };

    template <typename charT, typename WriterT>
    class LineWriter : public WriterT
    {
    public:
        typedef charT charT;

    public:
        void WriteLine(const charT *s, size_t len)
        {
            Write(static_cast<const char *>(s), sizeof(charT) * len);
        }
    };

    template <typename charT>
    class LineFileWriter : public LineWriter<charT, FileWriter>
    {
    };

    template <typename LineReaderT>
    class BufferReader : public LineReaderT
    {
        void ProcessLine(const char *s, size_t len)
        {
            LineReaderT::ReadLine(static_cast<const charT *>(s), len / sizeof(charT));
        }
    };
}