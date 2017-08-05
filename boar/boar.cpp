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
        // 1059203072      2068549
        // 36,762,348,544 bytes.
        // AMD E2-7110
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
        // 1059203072      1720807
        // 36,762,348,544 bytes.
        // AMD E2-7110
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
        // Too slow to finish.
        // 36,762,348,544 bytes.
        // AMD E2-7110
        size_t lineCount;
        FileSource(fileName, [&lineCount](const void *addr, size_t n) {
            const char* p = reinterpret_cast<const char*>(addr);
            size_t c = 0;
#if 1
#pragma omp parallel for reduction(+:c)
            for (intptr_t i = 0; i < static_cast<intptr_t>(n); i++)
            {
                if (p[i] == '\n') c++;
            }
#else
#define CS (1024*1024)
#pragma omp parallel for reduction(+:c)
            for (intptr_t i = 0; i < n; i += CS)
            {
                size_t d = 0;
                for (intptr_t j = i; j < i + CS && j < n; j++)
                    if (p[j] == '\n') d++;
                c += d;
            }
#endif
            lineCount = c;
        });
        return lineCount;
    }

    size_t LineCountRef3(const boost::filesystem::path fileName)
    {
        // 1059203070      463179
        // 36,762,348,544 bytes.
        // AMD E2-7110
        size_t lineCount = 0;
        FileSource2(fileName, [&lineCount](const void *addr, size_t n) {
            const char* p = reinterpret_cast<const char*>(addr);
            size_t c = 0;
            for (size_t i = 0; i < n; i++)
            {
                if (p[i] == '\n') c++;
            }
            lineCount += c;
        });
        return lineCount;
    }

    size_t LineCountRef4(const boost::filesystem::path fileName)
    {
        // 1059203072      414019
        // 36,762,348,544 bytes.
        // AMD E2-7110
        size_t lineCount = 0;
        FileSource3(fileName, [&lineCount](const void *addr, size_t n) {
            const char* p = reinterpret_cast<const char*>(addr);
            size_t c = 0;
            for (size_t i = 0; i < n; i++)
            {
                if (p[i] == '\n') c++;
            }
            lineCount += c;
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

    int Main(const std::vector<std::wstring>& args)
    {
        const boost::filesystem::path fileName(args[1].c_str());
        if (args[0] == L"1")
        {
            DumpProfile([&fileName]() { return LineCount(fileName); });
        }
        else if (args[0] == L"2")
        {
            DumpProfile([&fileName]() { return LineCountRef(fileName); });
        }
        else if (args[0] == L"3")
        {
            DumpProfile([&fileName]() { return LineCountRef2(fileName); });
        }
        else if (args[0] == L"4")
        {
            DumpProfile([&fileName]() { return LineCountRef3(fileName); });
        }
        else if (args[0] == L"5")
        {
            DumpProfile([&fileName]() { return LineCountRef4(fileName); });
        }
#if false
        for (int i = 0; i < 30; i++)
        {
            const boost::filesystem::path fileName(L"C:\\Users\\katsuya\\Source\\Repos\\CNTK\\Examples\\SequenceToSequence\\CMUDict\\Data\\cmudict-0.7b.train-dev-20-21.ctf");
            DumpProfile([&fileName]() { return LineCount(fileName); });
            DumpProfile([&fileName]() { return LineCountRef(fileName); });
            DumpProfile([&fileName]() { return LineCountRef2(fileName); });
        }
#endif
        return 0;
    }
}
