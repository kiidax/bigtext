/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <stdint.h>

namespace boar {
    typedef unsigned char char8_t;
    typedef std::basic_string<char8_t> c8string;
    template<typename charT> class Cursor;

    template<typename charT> class L2Node
    {
        // A buffer node to hold texts, which can hold about 1m Unicode chars.
    private:
        friend class Cursor<charT>;
        charT* _data;
        size_t _length;

    public:
        L2Node() {}
        ~L2Node() {}
    };

    template<typename charT> class L1Node
    {
        // A buffer node to hold texts, which can hold about 1m Unicode chars.
    private:
        friend class Cursor<charT>;
        // This can hold 1k nodes.
        std::vector<L2Node<charT>> _children;

    public:
        L1Node() {}
        ~L1Node() {}
    };

    template<typename charT> class Buffer
    {
    private:
        friend class Cursor<charT>;
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
        charT* _gapStart;
        charT* _gapEnd;

    public:
        Cursor(Buffer<charT>& buffer) : _buffer(buffer), _l1node(), _l2node(),
            _gapStart(), _gapEnd()
        {
        }
        ~Cursor() {}
        void Append(const charT* start, const charT* end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();

    private:
        void _CloseNode();
    };

    //template<> void Cursor<char8_t>::Insert(const char8_t* start, const char8_t* end);

    int Main(const std::vector<std::u16string>& args);
}