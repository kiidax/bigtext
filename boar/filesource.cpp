/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "filesource.h"

namespace boar
{
    typedef std::function<void(const void*, size_t)> DataSourceCallbackType;

    void FileSource(const boost::filesystem::path& filename, DataSourceCallbackType f)
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

#define CS (64 * 1024)

    void FileSource2(const boost::filesystem::path& filename, DataSourceCallbackType f)
    {
        bool success = false;
        LPCWSTR lpFileName = filename.native().c_str();
        HANDLE hFile = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BYTE buf[CS];
            DWORD readBytes;
            while (true)
            {
                if (!ReadFile(hFile, buf, CS, &readBytes, NULL))
                    break;
                if (readBytes == 0)
                    break;
                f(buf, readBytes);
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

    void FileSource3(const boost::filesystem::path& filename, DataSourceCallbackType f)
    {
        bool success = false;
        LPCWSTR lpFileName = filename.native().c_str();
        HANDLE hFile = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            BYTE buf[4][CS];
            OVERLAPPED ol[4];
            DWORD readBytes;
            int processIndex = 0;
            int numWaiting = 0;
            HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("test"));
            ZeroMemory(&buf, sizeof buf);
            size_t offset = 0;
            while (true)
            {
                if (numWaiting < 4)
                {
                    int readIndex = (processIndex + numWaiting) % 4;
                    ZeroMemory(&ol[readIndex], sizeof ol[readIndex]);
                    ol[readIndex].hEvent = hEvent;
                    ol[readIndex].Offset = static_cast<DWORD>(offset);
                    ol[readIndex].OffsetHigh = static_cast<DWORD>(offset >> 32);
                    offset += CS;
                    if (ReadFile(hFile, buf[readIndex], CS, NULL, &ol[readIndex]) || GetLastError() != ERROR_IO_PENDING)
                    {
                        std::cout << "what is this?" << std::endl;
                        break;
                    }
                    //std::cout << "read" << std::endl;
                    numWaiting++;
                }
                else
                {
                    while (true)
                    {
                        if (!GetOverlappedResult(hFile, &ol[processIndex], &readBytes, TRUE))
                        {
                            if (GetLastError() == ERROR_HANDLE_EOF)
                            {
                                success = true;
                                std::cout << "success" << std::endl;
                            }
                            else
                            {
                                std::cout << "????" << std::endl;
                            }
                            break;
                        }
                        if (readBytes == 0)
                        {
                            std::cout << ol[processIndex].Offset << std::endl;
                            std::cout << ol[processIndex].OffsetHigh << std::endl;
                            std::cout << readBytes << std::endl;
                        }
                        else
                        {
                            break;
                        }
                    }
                    if (success)
                        break;
                    //std::cout << "process" << std::endl;
                    f(buf[processIndex], readBytes);
                    processIndex = (processIndex + 1) % 4;
                    numWaiting--;
                }
            }
            CloseHandle(hFile);
        }
        if (!success)
        {
            DWORD dwErrorCode = GetLastError();
            TCHAR buf[1024];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode, 0, buf, sizeof buf, NULL);
            std::cout << "error" << std::endl;
            std::wstring s(reinterpret_cast<wchar_t*>(buf));
            std::wcout << s.substr(0, s.length() - 2) << std::endl;
        }
    }
}