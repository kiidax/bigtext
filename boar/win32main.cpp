/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

int _tmain(int argc, TCHAR** argv)
{
    std::setlocale(LC_CTYPE, "");
    std::vector<std::wstring> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(std::wstring(argv[i]));
    }
    return boar::Main(args);
}
