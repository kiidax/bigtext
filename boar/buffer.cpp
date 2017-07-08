/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#include "stdafx.h"

#include "boar.h"
#include "buffer.h"

namespace boar
{
    template<>
    void Buffer<char>::Open(const char16_t* fileName)
    {
        std::ifstream fin;
        fin.open((char*)FromUnicode(fileName).c_str());
        if (fin.is_open())
        {
            std::string line;
            while (std::getline(fin, line))
            {
                std::string linenl(line.begin(), line.end());
                linenl += _lineSeparator;
                insert(linenl.c_str(), linenl.c_str() + linenl.length());
            }
        }
    }

    template<>
    void Buffer<wchar_t>::Open(const char16_t* fileName)
    {
        std::ifstream fin;
        fin.open((char*)FromUnicode(fileName).c_str());
        if (fin.is_open())
        {
            std::string line;
            while (std::getline(fin, line))
            {
                std::wstring line(line.begin(), line.end());
                line += _lineSeparator;
                insert(line.c_str(), line.c_str() + line.length());
            }
        }
    }

    template<>
    void Buffer<char>::Dump()
    {
        for (auto it = _nodeList.begin(); it != _nodeList.end(); ++it)
        {
            std::cout << ">>>> Items: " << it->size();
            std::string str(it->begin(), it->end());
            int len = str.size();
            if (len > 30) len = 30;
            str.resize(len);
            std::cout << " " << str << std::endl;
        }
    }
}
