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
        FileSourceWithMemoryMapping(fileName, [&queue](const void* addr, size_t n) {
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
        FileSourceWithMemoryMapping(fileName, [&lineCount](const void *_addr, size_t n) {
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

    size_t LineCountRef3(const boost::filesystem::path fileName)
    {
        // 1059203070      463179
        // 36,762,348,544 bytes.
        // AMD E2-7110
        size_t lineCount = 0;
        FileSourceWithFileRead(fileName, [&lineCount](const void *addr, size_t n) {
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
        FileSourceWithOverlapRead(fileName, [&lineCount](const void *addr, size_t n) {
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

    int DumpProfile(std::function<size_t()> func)
    {
        clock_t startTime = clock();
        size_t lineCount = func();
        clock_t endTime = clock();
        clock_t t = endTime - startTime;
        std::cout << lineCount << '\t' << t << std::endl;
        return 0;
    }

    int Main(const std::vector<std::wstring>& args)
    {
        int status;
        if (args.size() == 2)
        {
            const boost::filesystem::path fileName(args[1].c_str());
            if (args[0] == L"1")
            {
                status = DumpProfile([&fileName]() { return LineCount(fileName); });
            }
            else if (args[0] == L"2")
            {
                status = DumpProfile([&fileName]() { return LineCountRef(fileName); });
            }
            else if (args[0] == L"3")
            {
                status = DumpProfile([&fileName]() { return LineCountRef3(fileName); });
            }
        }
        else if (args.size() == 1)
        {
            const boost::filesystem::path fileName(args[0].c_str());
            status = DumpProfile([&fileName]() { return LineCountRef4(fileName); });
        }
        else
        {
            status = 1;
        }
        return 0;
    }
}
