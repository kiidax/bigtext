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
    typedef boost::function<void (const void*, size_t)> TestWithFileCallbackType;

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

    size_t g = 0;

    clock_t startTime;
    clock_t endTime;
    boost::mutex test4mutex;
    boost::condition_variable test4cond;
    int i;
    void Test4a()
    {
        const boost::filesystem::path filename(L"C:\\Users\\katsuya\\Source\\Repos\\CNTK\\Examples\\SequenceToSequence\\CMUDict\\Data\\cmudict-0.7b.train-dev-20-21.ctf");
        TestWithFile(filename, [](const void* addr, size_t n) {
            TaskQueue queue;
            g = 0;
            i = 0;
            queue.Start();
            startTime = clock();
            const char* first = reinterpret_cast<const char*>(addr);
            const char* last = first + n;
            const size_t step = 64 * 1024;
            for (auto cur = first; cur < last; cur += step)
            {
                const char* cfirst = cur;
                const char* clast = min(cur + step, last);
                auto f = [cfirst, clast]() {
                    size_t count = 0;
                    for (auto cur = cfirst; cur < clast; ++cur)
                    {
                        if (*cur == '\n') ++count;
                    }
                    boost::mutex::scoped_lock lock(test4mutex);
                    g += count;
                    --i;
                    //std::cout << "local\t" << i << '\t' << (void*)cfirst << '\t' << (void*)clast << '\t' << count << std::endl;
                    if (i == 0)
                    {
                        test4cond.notify_one();
                        //std::cout << "done" << std::endl;
                    }
                };
                {
                    boost::mutex::scoped_lock lock(test4mutex);
                    ++i;
                }
                queue.PushTask(f);
            }
            while (true)
            {
                boost::mutex::scoped_lock lock(test4mutex);
                if (i != 0) 
                {
                    //std::cout << "wait" << std::endl;
                    test4cond.wait(lock);
                    //std::cout << "wake" << i << std::endl;
                }
                else
                {
                    break;
                }
            }
            endTime = clock();
        });
    }

    void Test4()
    {
        Test4a();
        clock_t t = endTime - startTime;
        std::cout << g << '\t' << t << std::endl;
    }

    int Main(const std::vector<std::u16string>& args)
    {
        Test4();
        return 0;
    }
}
