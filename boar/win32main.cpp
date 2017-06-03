/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"

using namespace std;
using namespace boar;

int _tmain(int argc, TCHAR** argv)
{
    vector<u16string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(u16string((char16_t*)argv[i]));
    }
    return Main(args);
}

