/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include <boar/base.h>

namespace boar {
    int Main(int argc, wchar_t *argv[]);
    int CountCommand(int argc, wchar_t *argv[]);
    int SampleCommand(int argc, wchar_t *argv[]);
}