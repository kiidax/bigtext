/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"
#include "filesource.h"
#include "ProcessorBase.h"

namespace boar
{
    ProcessorBase::ProcessorBase()
    {
    }

    ProcessorBase::~ProcessorBase()
    {
    }

    void ProcessorBase::ProcessFileList(const std::vector<std::wstring>& filePathList)
    {
        for (auto it = filePathList.cbegin(); it != filePathList.cend(); ++it)
        {
            ProcessFile(*it);
        }
    }

    void ProcessorBase::ProcessFile(const std::wstring& filePath)
    {
        _currentFilePath = filePath;
        BeginContent();
        FileSourceWithOverlapRead(filePath, [this](const void *addr, size_t n) {
            ProcessBuffer(addr, n);
        });
        EndContent();
    }

    void ProcessorBase::BeginContent()
    {
    }

    void ProcessorBase::EndContent()
    {
    }
}