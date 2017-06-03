/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"

namespace boar
{
    using namespace std;

    int Main(const vector<u16string>& args)
    {
        Buffer<char8_t> buffer;
        Cursor<char8_t> cursor(buffer);

        ifstream fin;
        fin.open("test.txt");
        if (fin.is_open())
        {
            string line;
            while (getline(fin, line))
            {
                c8string c8line(line.begin(), line.end());
                c8line += '\n';
                cursor.Append(c8line.c_str(), c8line.c_str() + c8line.length());
            }
        }

        cursor.MoveBeginningOfBuffer();
        while (true)
        {
            auto c8line = cursor.GetLineAndMoveNext();
            if (c8line.empty()) break;
            string line(c8line.begin(), c8line.end());
            std::cout << line << endl;
        }

        return 0;
    }
}
