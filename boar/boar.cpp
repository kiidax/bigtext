/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "buffer.h"

namespace boar
{
    int Main(const std::vector<std::u16string>& args)
    {
        Buffer<wchar_t> buffer;
        buffer.Open(args[0].c_str());

        buffer.MoveBeginningOfBuffer();
        while (true)
        {
            auto line = buffer.GetLineAndMoveNext();
            if (line.empty()) break;
            std::wcout << line << std::endl;
        }

        return 0;
    }
}
