/* Boar - Boar is a collection of tools to process text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

int wmain(int argc, wchar_t **argv)
{
    std::setlocale(LC_CTYPE, "");
    return boar::Main(argc, argv);
}

namespace boar
{
    uintmax_t GetPhysicalMemorySize()
    {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(memInfo);
        if (GlobalMemoryStatusEx(&memInfo))
        {
            uintmax_t res = static_cast<uintmax_t>(memInfo.ullTotalPhys);
            std::wcout << "\tPhysicalMemorySize\t" << res << std::endl;
#if !_WIN64
            if (res >= 1 << 31) res = 1 << 31;
#endif
            return res;
        }
        return 0;
    }
}
