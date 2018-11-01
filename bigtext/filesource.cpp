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

    void file_source_with_memory_mapping(const fs::path &file_name, data_source_callback callback)
    {
        boost::iostreams::mapped_file_source file;
        file.open(file_name);
        if (file.is_open()) {
            const char *s = file.data();
            size_t size = file.size();
            callback(s, size);
        }
        else
        {
            std::wcerr << __wcserror(file_name.native().c_str());
        }
    }

    void file_source_with_file_read(const fs::path& file_name, data_source_callback f)
    {
        bool success = false;
        LPCWSTR lpfile_name = file_name.native().c_str();
        HANDLE h_file = CreateFileW(lpfile_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
        if (h_file != INVALID_HANDLE_VALUE)
        {
            BYTE *buf = reinterpret_cast<BYTE *>(::VirtualAlloc(NULL, CHUNK_SIZE2, MEM_COMMIT, PAGE_READWRITE));
            if (buf != nullptr)
            {
                DWORD read_bytes;
                while (true)
                {
                    if (!ReadFile(h_file, buf, CHUNK_SIZE2, &read_bytes, NULL))
                        break;
                    if (read_bytes == 0)
                    {
                        success = true;
                        break;
                    }

                    f(reinterpret_cast<const char *>(buf), read_bytes);
                }
                ::VirtualFree(reinterpret_cast<LPVOID>(buf), 0, MEM_RELEASE);
            }
            CloseHandle(h_file);
        }
        if (!success)
        {
            DWORD dw_error_code = GetLastError();
            TCHAR buf[1024];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw_error_code, 0, buf, sizeof buf, NULL);
            std::wstring s(reinterpret_cast<wchar_t*>(buf));
            std::wcout << s.substr(0, s.length() - 2) << std::endl;
        }
    }

    void file_source_with_overlap_read(const fs::path& file_name, data_source_callback callback, uintmax_t max_size)
    {
        bool success = false;
        LPCWSTR lpfile_name = file_name.native().c_str();
        HANDLE h_file = CreateFileW(lpfile_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);
        if (h_file != INVALID_HANDLE_VALUE)
        {
            BYTE* buf = reinterpret_cast<BYTE *>(::VirtualAlloc(NULL, NUM_OVERLAPS * CHUNK_SIZE, MEM_COMMIT, PAGE_READWRITE));
            if (buf != nullptr)
            {
                OVERLAPPED ol[NUM_OVERLAPS];
                int process_index = 0;
                int num_waiting = 0;
                uintmax_t offset = 0;
                while (true)
                {
                    if (num_waiting < NUM_OVERLAPS && (max_size == 0 || offset < max_size))
                    {
                        int read_index = (process_index + num_waiting) % NUM_OVERLAPS;
                        ZeroMemory(&ol[read_index], sizeof ol[read_index]);
                        ol[read_index].Offset = static_cast<DWORD>(offset);
                        ol[read_index].OffsetHigh = static_cast<DWORD>(offset >> 32);
                        offset += CHUNK_SIZE;
                        if (ReadFile(h_file, buf + read_index * CHUNK_SIZE, CHUNK_SIZE, NULL, &ol[read_index]) || GetLastError() != ERROR_IO_PENDING)
                        {
                            break;
                        }
                        num_waiting++;
                    }
                    if (num_waiting == 0)
                    {
                        assert(max_size > 0);
                        assert(offset >= max_size);
                        success = true;
                        break;
                    }
                    else
                    {
                        DWORD read_bytes;
                        while (true)
                        {
                            if (!GetOverlappedResult(h_file, &ol[process_index], &read_bytes, TRUE))
                            {
                                read_bytes = 0;
                                if (GetLastError() == ERROR_HANDLE_EOF)
                                {
                                    callback(nullptr, 0);
                                    success = true;
                                }
                                break;
                            }
                            if (read_bytes > 0)
                            {
                                assert(read_bytes <= CHUNK_SIZE);
                                break;
                            }
                        }
                        if (read_bytes == 0)
                            break;
                        callback(reinterpret_cast<const char *>(buf + process_index * CHUNK_SIZE), read_bytes);
                        process_index = (process_index + 1) % NUM_OVERLAPS;
                        num_waiting--;
                    }
                }
                ::VirtualFree(reinterpret_cast<LPVOID>(buf), 0, MEM_RELEASE);
            }
            CloseHandle(h_file);
        }
        if (!success)
        {
            DWORD dw_error_code = GetLastError();
            TCHAR buf[1024];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw_error_code, 0, buf, sizeof buf, NULL);
            std::wstring s(reinterpret_cast<wchar_t*>(buf));
            std::wcerr << s.substr(0, s.length() - 2) << std::endl;
        }
    }

    void file_source_default(const fs::path& file_name, data_source_callback callback, uintmax_t max_size)
    {
        file_source_with_overlap_read(file_name, callback, max_size);
    }
}