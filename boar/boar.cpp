/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "buffer.h"

namespace boar
{
    void Test_GapVector()
    {
        GapVector<wchar_t> v1;
        GapVector<wchar_t> v2;
        std::wstring s1(L"Hello World!");
        v1.Insert(s1.cbegin(), s1.cend(), 0);
        std::wstring s2(L",");
        v1.Insert(s2.cbegin(), s2.cend(), 5);
        v1.SplitInto(3, v2);
    }

    int Main(const std::vector<std::u16string>& args)
    {
        Test_GapVector();
        Buffer<char> buffer;
        buffer.Open(args[0].c_str());
        buffer.Test();
        buffer.Dump();
        buffer.MoveBeginningOfBuffer();
        while (true)
        {
            auto line = buffer.GetLineAndMoveNext();
            if (line.empty()) break;
            std::cout << line << std::endl;
        }

        return 0;
    }
}
