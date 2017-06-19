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
                Insert(linenl.c_str(), linenl.c_str() + linenl.length());
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
        BufferNode<char>* node = _startNode;
        do
        {
            std::cout << ">>>> Items: " << node->Size();
            std::string str(node->Begin(), node->End());
            int len = str.size();
            if (len > 30) len = 30;
            str.resize(len);
            std::cout << " " << str << std::endl;
            node = node->Next();
        } while (node != _startNode);
    }
}
