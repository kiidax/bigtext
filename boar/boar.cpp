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
        // Empty constructor.
        GapVector<wchar_t> v1;
        assert(v1.size() == 0);
        assert(v1.empty());

        // Insert to an empty vector.
        std::wstring s1(L"Hellorld!");
        v1.Insert(s1.cbegin(), s1.cend(), 0);
        assert(v1.size() == s1.size());
        assert(!v1.empty());
        std::wcout << v1 << std::endl;

        // Insert to the middle.
        std::wstring s2(L"o, W");
        v1.Insert(s2.cbegin(), s2.cend(), 4);
        assert(v1.size() == s1.size() + s2.size());
        std::wcout << v1 << std::endl;

        // Splitting before the gap.
        GapVector<wchar_t> v2;
        v1.SplitInto(2, v2);
        assert(v1.size() == 2);
        assert(v2.size() == s1.size() + s2.size() - 2);
        std::wcout << v1 << "***" << v2 << std::endl;

        // Splitting after the gap.
        GapVector<wchar_t> v3;
        v2.SplitInto(4, v3);
        assert(v2.size() == 4);
        assert(v3.size() == s1.size() + s2.size() - 2 - 4);
        std::wcout << v1 << "***" << v2 << "***" << v3 << std::endl;

        // Copy constructor.
        GapVector<wchar_t> v4(v2);
        assert(v4.size() == 4);
        std::wcout << v1 << "***" << v4 << "***" << v3 << std::endl;
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
