/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"

using namespace boar;

int _tmain(int argc, TCHAR** argv)
{
    std::vector<std::u16string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(std::u16string((char16_t*)argv[i]));
    }
    return Main(args);
}
