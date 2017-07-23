/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>

namespace boar
{
    static void Test()
    {
        namespace fs = boost::filesystem;

        fs::path path("/local/boost_1_64_0/boost/filesystem");
        //const fs::path path("C:\\local\\boost_1_64_0\\boost\\filesystem");
        auto e = path.make_preferred();
        std::wcout << e << std::endl;
        std::wstring x = path.native();
        std::wcout << x << std::endl;
        auto parent_path = path.parent_path();
        std::wcout << parent_path << std::endl;

        BOOST_FOREACH(const fs::path& p, std::make_pair(fs::recursive_directory_iterator(path),
            fs::recursive_directory_iterator())) {
            if (!fs::is_directory(p))
                std::cout << p << std::endl;
        }
    }

    static void Test2()
    {
        bool success = false;
        HANDLE hFile = CreateFile(_T("test.txt"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BYTE buf[1024];
            DWORD bytesRead;
            BOOL res = ReadFile(hFile, buf, sizeof buf, &bytesRead, NULL);
            std::cout << std::string(reinterpret_cast<char*>(buf), bytesRead) << std::endl;

            HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (hMapping != NULL)
            {
                VOID* lpAddress = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
                if (lpAddress != NULL)
                {
                    MEMORY_BASIC_INFORMATION mbi;
                    if (VirtualQuery(lpAddress, &mbi, sizeof mbi) != 0)
                    {
                        std::cout << mbi.RegionSize << std::endl;
                        std::string s(reinterpret_cast<char*>(lpAddress), mbi.RegionSize);
                        std::cout << s << std::endl;
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

    typedef boost::function<void (const void*, size_t)> TestWithFileCallbackType;

    void TestWithFile(TestWithFileCallbackType f)
    {
        bool success = false;
        const wchar_t* filename = L"C:\\Users\\katsuya\\Source\\Repos\\CNTK\\Examples\\SequenceToSequence\\CMUDict\\Data\\cmudict-0.7b.train-dev-20-21.ctf";
        HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

    void Test3(TestWithFileCallbackType f)
    {
        TestWithFile(f);
    }

    size_t g = 0;

    void some_computation(const char* first, const char* last)
    {
        int c = 0;
        for (const char* cur = first; cur < last; cur++)
        {
            if (*cur == '\n') c++;
        }
        g += c;
    }

    void Test3_1(const void* _addr, size_t n)
    {
        const char* addr = reinterpret_cast<const char*>(_addr);
        g = 0;
        size_t step = n / 4;
        boost::thread_group group;
        for (const char* cur = addr; cur < addr + n; cur += step)
        {
            group.create_thread(boost::bind(&some_computation, cur, min(cur + step, addr + n)));
        }
        group.join_all();
        std::cout << g << '\t';
    }

    void Test3_2(const void* _addr, size_t n)
    {
        const char* addr = reinterpret_cast<const char*>(_addr);
        g = 0;

        int c = 0;
        for (const char* cur = addr; cur < addr + n; cur++)
        {
            if (*cur == '\n') c++;
        }
        g += c;

        std::cout << g << std::endl;
    }

    class Job
    {
    public:
        typedef boost::function<bool(int, int)> CallbackT;
    private:
//        const void* _firstAddr;
  //      const void* _lastAddr;
        CallbackT _callback;
    public:
        Job(CallbackT callback) : _callback(callback) {}
        void Start() {
            //assert(_firstAddr != nullptr);
            //assert(_lastAddr != nullptr);
            _callback(1, 2);
        }
        //virtual void Run() = 0;
    };

    void Test3a()
    {
        for (int i = 0; i < 100; i++)
        {
            clock_t t = clock();
            if (i % 2 == 0)
            {
                Test3(Test3_1);
            }
            else
            {
                Test3(Test3_2);
            }
            clock_t e = clock();
            std::cout << (e - t) << std::endl;
        }
    }

    class ThreadPool
    {
    private:
        typedef boost::function<void(void)> JobType;
        boost::mutex _mutex;
        boost::condition_variable _condEmpty;
        boost::condition_variable _condFull;
        boost::circular_buffer<JobType> _buffer;
        boost::thread_group _threadGroup;
        int _numThreads;
        bool _done;

    public:
        ThreadPool(int numThreads, int maxJobs) : _buffer(maxJobs), _numThreads(numThreads), _done(false)
        {
        }
        ~ThreadPool()
        {
            {
                boost::mutex::scoped_lock lock(_mutex);
                std::cout << "finish" << std::endl;
                _done = true;
                _condEmpty.notify_all();
            }
            _threadGroup.join_all();
        }
        void Start()
        {
            for (int i = 0; i < _numThreads; i++)
            {
                _threadGroup.create_thread([this]() {
                    while (true)
                    {
                        JobType job;
                        if (!_GetJob(job)) break;
                        job();
                    }
                });
            }
        }
        bool _GetJob(JobType& job)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (!_done && _buffer.empty())
                {
                    _condEmpty.wait(lock);
                }
                else
                {
                    if (_done) return false;
                    job = _buffer[0];
                    bool doNotify = _buffer.full();
                    _buffer.pop_front();
                    if (doNotify) _condFull.notify_one();
                    return true;
                }
            }
        }
        void PostJob(JobType job)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                _condFull.wait(lock);
            }
            bool doNotify = _buffer.empty();
            _buffer.push_back(job);
            if (doNotify) _condEmpty.notify_one();
        }
    };

    clock_t startTime;
    clock_t endTime;
    boost::mutex test4mutex;
    boost::condition_variable test4cond;
    int i;
    void Test4a()
    {
        TestWithFile([](const void* addr, size_t n) {
            ThreadPool pool(5, 100);
            g = 0;
            i = 0;
            pool.Start();
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
                pool.PostJob(f);
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
            std::cout << "wake" << i << std::endl;
            endTime = clock();
        });
    }

    void Test4b()
    {
        TestWithFile([](const void* addr, size_t n) {
        });
    }

    void Test4()
    {
        for (int i = 0; i < 1; i++)
        {
            if (i % 2 == 0)
            {
                Test4a();
            }
            else
            {
                Test4a();
            }
            clock_t t = endTime - startTime;
            std::cout << g << '\t' << t << std::endl;
        }
    }

    int Main(const std::vector<std::u16string>& args)
    {
        Test4();
        return 0;
    }
}
