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

    struct ChunkInfo
    {
    public:
        size_t lineCount;
    };

    class TestClass
    {
    private:
        TaskQueue& _queue;
        boost::circular_buffer<ChunkInfo> _tasks;

        size_t lineCount = 0;
        clock_t startTime;
        clock_t endTime;
        boost::mutex _mutex;
        boost::condition_variable test4cond;
        static const size_t CHUNK_SIZE = 64 * 1024;
        int i;

    public:
        TestClass(TaskQueue& queue)
            : _queue(queue) {}
        void LineCount(const boost::filesystem::path& fileName)
        {
            TestWithFile(fileName, [this](const void* addr, size_t n) {
                lineCount = 0;
                i = 0;
                _queue.Start();
                startTime = clock();
                const char* first = reinterpret_cast<const char*>(addr);
                const char* last = first + n;
                for (auto cur = first; cur < last; cur += CHUNK_SIZE)
                {
                    const char* cfirst = cur;
                    const char* clast = min(cur + CHUNK_SIZE, last);
                    ChunkInfo info;
                    //_buffer.push_back(info);
                    auto f = [this, cfirst, clast]() {
                        size_t count = 0;
                        for (auto cur = cfirst; cur < clast; ++cur)
                        {
                            if (*cur == '\n') ++count;
                        }
                        boost::mutex::scoped_lock lock(_mutex);
                        lineCount += count;
                        --i;
                        if (i == 0)
                        {
                            test4cond.notify_one();
                        }
                    };
                    {
                        boost::mutex::scoped_lock lock(_mutex);
                        ++i;
                    }
                    TaskInfo task;
                    task.func = f;
                    task.done = false;
                    _queue.PushTask(task);
                }
                while (true)
                {
                    boost::mutex::scoped_lock lock(_mutex);
                    if (i != 0)
                    {
                        test4cond.wait(lock);
                    }
                    else
                    {
                        break;
                    }
                }
                _queue.Stop();
                endTime = clock();
            });
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
            TaskQueue queue;
            TestClass test(queue);
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
