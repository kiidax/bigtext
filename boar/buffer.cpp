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
                std::string line(line.begin(), line.end());
                line += _lineSeparator;
                Insert(line.c_str(), line.c_str() + line.length());
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
                Insert(line.c_str(), line.c_str() + line.length());
            }
        }
    }

    template<>
    void Buffer<char>::Dump()
    {
        auto& node2 = _root;
        std::cout << "Items: " << node2.Size() << std::endl;
        for (auto it2 = _root.Begin(); it2 != _root.End(); ++it2)
        {
            auto& node1 = (*it2);
            std::cout << ">> Items: " << node1.Size() << std::endl;
            for (auto it1 = node1.Begin(); it1 != node1.End(); ++it1)
            {
                auto& node0 = (*it1);
                std::cout << ">>>> Items: " << node0.Size();
                std::string str(node0.Begin(), node0.End());
                int len = str.size();
                if (len > 30) len = 30;
                str.resize(len);
                std::cout << " " << str << std::endl;
            }
        }
    }
}
