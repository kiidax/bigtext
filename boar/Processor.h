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
    };
}
