/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "taskqueue.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

namespace boar
{
    typedef std::function<void (const void*, size_t)> TestWithFileCallbackType;

    void TestWithFile(const boost::filesystem::path& filename, TestWithFileCallbackType f)
    {
        bool success = false;
        LPCWSTR lpFileName = filename.native().c_str();
        HANDLE hFile = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (hMapping != NULL)
            {
                VOID* lpAddress = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
                if (lpAddress != NULL)
                {
                    MEMORY_BASIC_INFORMATION mbi;
                    if (VirtualQuery(lpAddress, &mbi, sizeof mbi) != 0)
                    {
                        f(lpAddress, mbi.RegionSize);
                        success = true;
                    }
                    UnmapViewOfFile(lpAddress);
                }
                CloseHandle(hMapping);
            }
            CloseHandle(hFile);
        }
        if (!success)
        {
            DWORD dwErrorCode = GetLastError();
            TCHAR buf[1024];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, buf, sizeof buf, NULL);
            std::wstring s(reinterpret_cast<wchar_t*>(buf));
            std::wcout << s.substr(0, s.length() - 2) << std::endl;
        }
    }

    class TestClass
    {
    private:
        size_t lineCount = 0;
        clock_t startTime;
        clock_t endTime;
        static const size_t CHUNK_SIZE = 64 * 1024;

    public:
        TestClass() {}
        void LineCount(const boost::filesystem::path& fileName)
        {
            startTime = clock();
            auto taskFunc = [this](void* addr, size_t n, size_t& count)
            {
                size_t c = 0;
                const char* p = reinterpret_cast<const char*>(addr);
                for (auto cur = p; cur < p + n; ++cur)
                {
                    if (*cur == '\n') ++c;
                }
                count = c;
            };
            auto reduceFunc = [this](size_t count, void* addr, size_t n)
            {
                lineCount += count;
            };
            TaskQueue queue(taskFunc, reduceFunc);
            queue.Start();
            TestWithFile(fileName, [this, &queue](const void* addr, size_t n) {
                lineCount = 0;
                for (size_t i = 0; i < n; i += CHUNK_SIZE)
                {
                    intptr_t chunkAddr = reinterpret_cast<intptr_t>(addr) + i;
                    size_t chunkSize = n - i < CHUNK_SIZE ? n - i : CHUNK_SIZE;
                    queue.AddTask(reinterpret_cast<void*>(chunkAddr), chunkSize);
                }
                queue.Synchronize();
            });
            queue.Stop();
            endTime = clock();
        }
        void DumpProfile()
        {
            clock_t t = endTime - startTime;
            std::cout << lineCount << '\t' << t << std::endl;
        }
    };

    int Main(const std::vector<std::u16string>& args)
    {
        for (int i = 0; i < 30; i++)
        {
            TestClass test;
            const boost::filesystem::path fileName(L"C:\\Users\\katsuya\\Source\\Repos\\CNTK\\Examples\\SequenceToSequence\\CMUDict\\Data\\cmudict-0.7b.train-dev-20-21.ctf");
            test.LineCount(fileName);
            test.DumpProfile();

            {
                TestWithFile(fileName, [](const void *_addr, size_t n) {
                    clock_t startTime = clock();
                    const char* first = reinterpret_cast<const char*>(_addr);
                    const char* end = reinterpret_cast<const char*>(_addr) + n;
                    int lineCount = 0;
                    for (auto cur = first; cur != end; ++cur)
                    {
                        if (*cur == '\n') lineCount++;
                    }
                    clock_t endTime = clock();
                    clock_t t = endTime - startTime;
                    std::cout << lineCount << '\t' << t << std::endl;
                });
            }
        }
        return 0;
    }
}
