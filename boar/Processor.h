/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    class Processor
    {
    protected:
        std::wstring _currentFilePath;
        std::wstring _outputFilePath;
        bool _forceOverWrite;
        std::ofstream _outf;

    public:
        Processor() : _forceOverWrite(false) {}
        virtual ~Processor() {}

        void SetOutputFilePath(const std::wstring& outputFilePath) { _outputFilePath = outputFilePath; }
        void ProcessFileList(const std::vector<std::wstring>& filePathList);
        void ProcessFile(const std::wstring& filePath);

    protected:
        virtual void BeginFile() {}
        virtual void ProcessBlock(_In_ const char* s, _In_ size_t len) = 0;
        virtual void EndFile() {}
    };
}
