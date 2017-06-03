/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

using namespace std;

int main()
{
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
                fout << line;
            }
        }
    }
    return 0;
}

