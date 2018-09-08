/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"

namespace boar
{
    namespace fs = std::filesystem;

    static const int NUM_OVERLAPS = 3;
    static const size_t CHUNK_SIZE = 64L * 1024;
    static const size_t CHUNK_SIZE2 = 64L * 1024;

    void FileSourceWithMemoryMapping(const fs::path& fileName, DataSourceCallback callback)
    {
        bool success = false;
        LPCWSTR lpfileName = fileName.native().c_str();
        HANDLE hFile = ::CreateFileW(lpfileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD highSize;
            DWORD lowSize = ::GetFileSize(hFile, &highSize);
            uint64_t size = (static_cast<uint64_t>(highSize) << 32) | lowSize;

            HANDLE hMapping = ::CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (hMapping != NULL)
            {
                VOID* lpAddress = ::MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
                if (lpAddress != NULL)
                {
                    MEMORY_BASIC_INFORMATION mbi;
                    if (::VirtualQuery(lpAddress, &mbi, sizeof mbi) != 0)
                    {
                        const char *s = reinterpret_cast<const char *>(lpAddress);
                        if (size <= mbi.RegionSize)
                        {
                            callback(s, static_cast<size_t>(size));
                            success = true;
                        }
                    }
                    ::UnmapViewOfFile(lpAddress);
                }
                ::CloseHandle(hMapping);
            }
            ::CloseHandle(hFile);
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