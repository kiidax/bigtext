#include "stdafx.h"
#include "filesource.h"
#include "TextProcessor.h"

namespace boar
{
    TextProcessor::TextProcessor()
    {
    }

    TextProcessor::~TextProcessor()
    {
    }

    void TextProcessor::ProcessFileList(const std::vector<std::wstring>& filePathList)
    {
        for (auto it = filePathList.cbegin(); it != filePathList.cend(); ++it)
        {
            ProcessFile(*it);
        }
    }

    void TextProcessor::ProcessFile(const std::wstring& filePath)
    {
        BeginContent(filePath);
        FileSourceWithOverlapRead(filePath, [this](const void *addr, size_t n) {
            ProcessBuffer(addr, n);
        });
        EndContent();
    }

    void TextProcessor::BeginContent(const std::wstring& filePath)
    {
        _currentFilePath = filePath;
    }

    void TextProcessor::EndContent()
    {
    }
}