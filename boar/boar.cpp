/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "buffer.h"

namespace boar
{
    std::wostream& operator << (std::wostream& os, GapVector<wchar_t>& v)
    {
        os << std::wstring(v.begin(), v.end());
        return os;
    }

    void Test_GapVector()
    {
        GapVector<wchar_t> v1;
        assert(v1.size() == 0);
        assert(v1.empty());
        GapVector<wchar_t> v2;
        std::wstring s1(L"Hellold!");
        v1.Insert(s1.cbegin(), s1.cend(), 0);
        assert(v1.size() == s1.size());
        assert(!v1.empty());
        std::wcout << v1 << std::endl;
        std::wstring s2(L", Wor");
        v1.Insert(s2.cbegin(), s2.cend(), 5);
        std::wcout << v1 << std::endl;
        v1.SplitInto(3, v2);
        std::wcout << v1 << "***" << v2 << std::endl;
        GapVector<wchar_t> v3(v1);
        std::wcout << v3 << "***" << v2 << std::endl;
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
