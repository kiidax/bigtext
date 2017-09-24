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
        FileSourceWithOverlapRead(filePath, [this](const void* first, const void* last) {
            ProcessBuffer(first, last);
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