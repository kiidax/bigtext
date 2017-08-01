/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "taskqueue.h"
#include "filesource.h"

namespace boar
{
    size_t LineCount(const boost::filesystem::path& fileName)
    {
        size_t lineCount;
        auto taskFunc = [](void* addr, size_t n)
        {
            size_t c = 0;
            const char* p = reinterpret_cast<const char*>(addr);
            for (auto cur = p; cur < p + n; ++cur)
            {
                if (*cur == '\n') ++c;
            }
            return c;
        };
        auto reduceFunc = [&lineCount](size_t count, void* addr, size_t n)
        {
            lineCount += count;
        };
        TaskQueue queue(taskFunc, reduceFunc);
        queue.Start();
        lineCount = 0;
        FileSource(fileName, [&queue](const void* addr, size_t n) {
            queue.Dispatch(addr, n);
        });
        queue.Stop();
        return lineCount;
    }

    size_t LineCountRef(const boost::filesystem::path fileName)
    {
        size_t lineCount;
        FileSource(fileName, [&lineCount](const void *_addr, size_t n) {
            const char* first = reinterpret_cast<const char*>(_addr);
            const char* end = reinterpret_cast<const char*>(_addr) + n;
            size_t c = 0;
            for (auto cur = first; cur != end; ++cur)
            {
                if (*cur == '\n') c++;
            }
            lineCount = c;
        });
        return lineCount;
    }

    size_t LineCountRef2(const boost::filesystem::path fileName)
    {
        size_t lineCount;
        FileSource(fileName, [&lineCount](const void *addr, size_t n) {
            const char* p = reinterpret_cast<const char*>(addr);
            size_t c = 0;
#pragma omp parallel for reduction(+:c)
            for (intptr_t i = 0; i < n; i++)
            {
                if (p[i] == '\n') c++;
            }
            lineCount = c;
        });
        return lineCount;
    }

    void DumpProfile(std::function<size_t()> func)
    {
        clock_t startTime = clock();
        size_t lineCount = func();
        clock_t endTime = clock();
        clock_t t = endTime - startTime;
        std::cout << lineCount << '\t' << t << std::endl;
    }

    int Main(const std::vector<std::u16string>& args)
    {
        for (int i = 0; i < 30; i++)
        {
            const boost::filesystem::path fileName(L"C:\\Users\\katsuya\\Source\\Repos\\CNTK\\Examples\\SequenceToSequence\\CMUDict\\Data\\cmudict-0.7b.train-dev-20-21.ctf");
            DumpProfile([&fileName]() { return LineCount(fileName); });
            DumpProfile([&fileName]() { return LineCountRef(fileName); });
            DumpProfile([&fileName]() { return LineCountRef2(fileName); });
        }
        return 0;
    }
}
