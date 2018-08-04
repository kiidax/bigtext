/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"
#include "filesource.h"
#include "Processor.h"

namespace boar
{
    void Processor::ProcessFileList(const std::vector<std::wstring>& filePathList)
    {
        if (!_delayOpenFile)
        {
            std::wcout << _outputFilePath << std::endl;
            _outf.open(_outputFilePath, std::ios::out | std::ios::binary);
        }
        for (auto it = filePathList.cbegin(); it != filePathList.cend(); ++it)
        {
            ProcessFile(*it);
        }
        if (_outf.is_open())
        {
            _outf.close();
        }
    }

    void Processor::ProcessFile(const std::wstring& filePath)
    {
        _currentFilePath = filePath;
        BeginFile();
        FileSourceWithOverlapRead(filePath, [this](const void* first, const void* last) {
            ProcessBlock(first, last);
        });
        if (_delayOpenFile)
        {
            //_outf.open(_outputFilePath);
        }
        EndFile();
    }

    void Processor::OutputBuffer(_In_ const void* first_, _In_ const void* last_)
    {
    }
}