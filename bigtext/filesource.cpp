/* bigtext - bigtext is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "bigtext.h"
#include "filesource.h"
#include <boost/iostreams/device/mapped_file.hpp>

namespace bigtext
{
    namespace fs = boost::filesystem;

    static const int NUM_OVERLAPS = 3;
    static const size_t CHUNK_SIZE = 64L * 1024;
    static const size_t CHUNK_SIZE2 = 64L * 1024;

    void FileSourceWithMemoryMapping(const fs::path &fileName, DataSourceCallback callback)
    {
        boost::iostreams::mapped_file_source file;
        file.open(fileName);
        if (file.is_open()) {
            const char *s = file.data();
            size_t size = file.size();
            callback(s, size);
        }
        else
        {
            std::wcerr << __wcserror(fileName.native().c_str());
        }
    }

    void FileSourceWithFileRead(const fs::path& fileName, DataSourceCallback f)
    {
        bool success = false;
        LPCWSTR lpfileName = fileName.native().c_str();
        HANDLE hFile = CreateFileW(lpfileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BYTE *buf = reinterpret_cast<BYTE *>(::VirtualAlloc(NULL, CHUNK_SIZE2, MEM_COMMIT, PAGE_READWRITE));
            if (buf != nullptr)
            {
                DWORD readBytes;
                while (true)
                {
                    if (!ReadFile(hFile, buf, CHUNK_SIZE2, &readBytes, NULL))
                        break;
                    if (readBytes == 0)
                    {
                        success = true;
                        break;
                    }

                    f(reinterpret_cast<const char *>(buf), readBytes);
                }
                ::VirtualFree(reinterpret_cast<LPVOID>(buf), 0, MEM_RELEASE);
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

    void FileSourceWithOverlapRead(const fs::path& fileName, DataSourceCallback callback, uintmax_t maxSize)
    {
        bool success = false;
        LPCWSTR lpfileName = fileName.native().c_str();
        HANDLE hFile = CreateFileW(lpfileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BYTE* buf = reinterpret_cast<BYTE *>(::VirtualAlloc(NULL, NUM_OVERLAPS * CHUNK_SIZE, MEM_COMMIT, PAGE_READWRITE));
            if (buf != nullptr)
            {
                OVERLAPPED ol[NUM_OVERLAPS];
                int processIndex = 0;
                int numWaiting = 0;
                uintmax_t offset = 0;
                while (true)
                {
                    if (numWaiting < NUM_OVERLAPS && (maxSize == 0 || offset < maxSize))
                    {
                        int readIndex = (processIndex + numWaiting) % NUM_OVERLAPS;
                        ZeroMemory(&ol[readIndex], sizeof ol[readIndex]);
                        ol[readIndex].Offset = static_cast<DWORD>(offset);
                        ol[readIndex].OffsetHigh = static_cast<DWORD>(offset >> 32);
                        offset += CHUNK_SIZE;
                        if (ReadFile(hFile, buf + readIndex * CHUNK_SIZE, CHUNK_SIZE, NULL, &ol[readIndex]) || GetLastError() != ERROR_IO_PENDING)
                        {
                            break;
                        }
                        numWaiting++;
                    }
                    if (numWaiting == 0)
                    {
                        assert(maxSize > 0);
                        assert(offset >= maxSize);
                        success = true;
                        break;
                    }
                    else
                    {
                        DWORD readBytes;
                        while (true)
                        {
                            if (!GetOverlappedResult(hFile, &ol[processIndex], &readBytes, TRUE))
                            {
                                readBytes = 0;
                                if (GetLastError() == ERROR_HANDLE_EOF)
                                {
                                    callback(nullptr, 0);
                                    success = true;
                                }
                                break;
                            }
                            if (readBytes > 0)
                            {
                                assert(readBytes <= CHUNK_SIZE);
                                break;
                            }
                        }
                        if (readBytes == 0)
                            break;
                        callback(reinterpret_cast<const char *>(buf + processIndex * CHUNK_SIZE), readBytes);
                        processIndex = (processIndex + 1) % NUM_OVERLAPS;
                        numWaiting--;
                    }
                }
                ::VirtualFree(reinterpret_cast<LPVOID>(buf), 0, MEM_RELEASE);
            }
            CloseHandle(hFile);
        }
        if (!success)
        {
            DWORD dwErrorCode = GetLastError();
            TCHAR buf[1024];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, buf, sizeof buf, NULL);
            std::wstring s(reinterpret_cast<wchar_t*>(buf));
            std::wcerr << s.substr(0, s.length() - 2) << std::endl;
        }
    }

    void FileSourceDefault(const fs::path& fileName, DataSourceCallback callback, uintmax_t maxSize)
    {
        FileSourceWithOverlapRead(fileName, callback, maxSize);
    }
}