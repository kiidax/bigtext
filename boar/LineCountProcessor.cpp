#include "stdafx.h"
#include "LineCountProcessor.h"

namespace boar
{
    LineCountProcessor::LineCountProcessor()
        : _lineCount()
    {
    }


    LineCountProcessor::~LineCountProcessor()
    {
    }


    void LineCountProcessor::ProcessBuffer(const void * data, size_t n)
    {
        const char* p = reinterpret_cast<const char*>(data);
        size_t c = 0;
        for (size_t i = 0; i < n; i++)
        {
            if (p[i] == '\n') c++;
        }
        _lineCount += c;
    }


    void LineCountProcessor::EndContent()
    {
        std::wcout << _currentFilePath << '\t' << _lineCount << std::endl;
    }
}
