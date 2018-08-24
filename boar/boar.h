/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include <boar/base.h>
#include <filesystem>

namespace boar {

    namespace fs = boost::filesystem;

    int Main(int argc, wchar_t *argv[]);
    int sample_command(int argc, wchar_t *argv[]);
    int SampleCommand(int argc, wchar_t *argv[]);

    class FileWriter
    {
    protected:
        fs::ofstream _out;

    public:
        void Open(const fs::path& path)
        {
            _out.open(path, std::ios::out | std::ios::binary);
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