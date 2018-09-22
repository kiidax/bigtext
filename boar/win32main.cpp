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
