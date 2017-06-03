/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <stdint.h>

namespace boar {
    typedef unsigned char char8_t;
    typedef std::basic_string<char8_t> c8string;

    template<typename charT> class L2Node
    {
        // A buffer node to hold texts, which can hold about 1m Unicode chars.
    private:
        std::vector<std::basic_string<charT>> _texts;
    public:
    };

    template<typename charT> class L1Node
    {
        // A buffer node to hold texts, which can hold about 1m Unicode chars.
    private:
        // This can hold 1k nodes.
        std::vector<L2Node<charT>> _children;
    public:
    };

    template<typename charT> class Buffer
    {
    private:
        // This can hold 1k nodes.
        std::vector<L1Node<charT>> _children;
    public:
        Buffer() {}
        ~Buffer() {}
    };

    template<typename charT> class Cursor
    {
    private:
        Buffer<charT>& _buffer;
        L1Node<charT>* _l1node;
        L2Node<charT>* _l2node;

    public:
        Cursor(Buffer<charT>& buffer) : _buffer(buffer)
        {
        }
        ~Cursor() {}
        void Insert(const charT* start, const charT* end);
    };

    int Main(const std::vector<std::u16string> & args);
}