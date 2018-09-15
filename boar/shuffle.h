/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2018 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "filesource.h"

namespace boar
{
    namespace fs = boost::filesystem;

    template<typename CharT, typename RangeT>
    void FileShuffleLines(const RangeT &inputFileNameList, const fs::path &outputFileName)
    {
        const fs::path &inputFileName = inputFileNameList[0];
        FileSourceWithBoostMemoryMapping(inputFileName, [&outputFileName](const char *_s, size_t _len) {
            LineFileWriter<CharT> out;
            out.Open(outputFileName);
            const CharT *s = reinterpret_cast<const CharT *>(_s);
            size_t len = _len / sizeof(CharT);
            std::vector<const CharT *> linePositionList;
            linePositionList.push_back(s);
            std::wcout << "File size is " << len << " chars." << std::endl;
            std::wcout << "Counting number of lines in the file." << std::endl;
            for (size_t i = 0; i < len; i++)
            {
                if (s[i] == '\n')
                {
                    linePositionList.push_back(&s[i + 1]);
                }
            }
            if (s[len - 1] != '\n')
            {
                linePositionList.push_back(&s[len]);
            }
            size_t numLines = linePositionList.size() - 1;
            std::vector<size_t> outputLines;
            for (size_t i = 0; i < numLines; i++)
            {
                outputLines.push_back(i);
            }
            for (size_t i = numLines - 1; i != 0; i--)
            {
                size_t j = std::rand() % (i + 1);
                std::swap(outputLines[i], outputLines[j]);
            }
            for (size_t n : outputLines)
            {
                const CharT *first = linePositionList[n];
                const CharT *last = linePositionList[n + 1];
                out.WriteLine(first, last - first);
            }
            out.Close();
        });
    }
}
