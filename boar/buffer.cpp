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
        std::cout << "Items: " << _root._children.size() << std::endl;
        auto children2 = _root._children;
        for (auto it2 = children2.begin(); it2 != children2.end(); ++it2)
        {
            std::cout << ">> Items: " << (*it2)._children.size() << std::endl;
            auto children1 = (*it2)._children;
            for (auto it1 = children1.begin(); it1 != children1.end(); ++it1)
            {
                std::cout << ">>>> Items: " << (*it1)._children.size();
                auto children0 = (*it1)._children;
                std::string str(children0.begin(), children0.end());
                int len = str.size();
                if (len > 30) len = 30;
                str.resize(len);
                std::cout << " " << str << std::endl;
            }
        }
    }
}
