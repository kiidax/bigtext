/* bigtext - bigtext is a collection of tools to process large text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "bigtext.h"

int wmain(int argc, wchar_t **argv)
{
    std::setlocale(LC_CTYPE, "");
    return bigtext::main(argc, argv);
}

namespace bigtext
{
    uintmax_t get_physical_memory_size()
    {
        MEMORYSTATUSEX mem_info;
        mem_info.dwLength = sizeof(mem_info);
        if (GlobalMemoryStatusEx(&mem_info))
        {
            uintmax_t res = static_cast<uintmax_t>(mem_info.ullTotalPhys);
            std::wcout << "\tPhysicalMemorySize\t" << res << std::endl;
#if !_WIN64
            if (res >= 1 << 31) res = 1 << 31;
#endif
            return res;
        }
        return 0;
    }
}
