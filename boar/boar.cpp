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
            ofstream fout;
            fout.open("out.txt");
            if (fout.is_open())
            {
                string line;
                while (getline(fin, line))
                {
                    c8string c8line(line.begin(), line.end());
                    cursor.Insert(c8line.c_str(), c8line.c_str() + c8line.length());
                    fout << line;
                }
            }
        }

        return 0;
    }

    template<>
    void Cursor<char8_t>::Insert(const char8_t* start, const char8_t* end)
    {
        std::cout << string(start, end) << std::endl;
    }
}
