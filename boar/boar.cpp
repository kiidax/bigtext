/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "buffer.h"
#include "cursor.h"

namespace boar
{
    int Main(const std::vector<std::u16string>& args)
    {
        Buffer<char> buffer;
        buffer.Open(args[0].c_str());
        Cursor<char> cursor(buffer);

        cursor.MoveBeginningOfBuffer();
        while (true)
        {
            auto line = cursor.GetLineAndMoveNext();
            if (line.empty()) break;
            std::cout << line << std::endl;
        }

        return 0;
    }
}
