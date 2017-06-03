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

    template<typename charT>
    void Cursor<charT>::Append(const charT* start, const charT* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);

        if (_l1node == nullptr)
        {
            assert(_l2node == nullptr);

            _buffer._children.push_back(L1Node<charT>());
            _l1node = &_buffer._children.back();
        }

        if (_l2node == nullptr)
        {
            assert(_gapStart == nullptr);
            assert(_gapEnd == nullptr);

            (*_l1node)._children.push_back(L2Node<charT>());
            _l2node = &(*_l1node)._children.back();
            charT* newData = (charT*)malloc(1024 * sizeof(charT));
            if (newData == nullptr) throw bad_alloc();

            _l2node->_data = newData;
            _l2node->_length = 1024;

            _gapStart = _l2node->_data;
            _gapEnd = _l2node->_data + _l2node->_length;
        }

        assert(_gapStart != nullptr);
        assert(_gapEnd != nullptr);

        if (_gapEnd - _gapStart < end - start)
        {
            size_t newLength = _l2node->_length + (end - start) - (_gapEnd - _gapStart);
            charT* newData = (charT*)realloc(_l2node->_data, newLength * sizeof(charT));
            if (newData == nullptr) throw bad_alloc();

            // Adjust the pointers.
            _gapStart = newData + (_gapStart - _l2node->_data);
            _gapEnd = newData + (_gapEnd - _l2node->_data);

            _l2node->_data = newData;
            _l2node->_length = newLength;
        }

        memcpy(_gapStart, start, (end - start) * sizeof(charT));
        _gapStart += end - start;
    }

    template<typename charT>
    std::basic_string<charT> Cursor<charT>::GetLineAndMoveNext()
    {
        if (_l1node == nullptr)
        {
            return std::basic_string<charT>();
        }

        charT *it;
        for (it = _gapStart; it != _l2node->_data + _l2node->_length; ++it)
        {
            if (*it == '\n')
                break;
        }

        std::basic_string<charT> ret(_gapStart, it - _gapStart);

        _gapStart = it + 1;
        _gapEnd = _gapEnd;
        if (_gapStart == _l2node->_data + _l2node->_length)
        {
            _l1node = nullptr;
            _l2node = nullptr;
            _gapStart = nullptr;
            _gapEnd = nullptr;
        }

        return ret;
    }

    template<typename charT>
    void Cursor<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _l1node = _buffer._children.data();
        _l2node = _l1node->_children.data();
        _gapStart = _l2node->_data + _l2node->_length;
        _gapEnd = _gapStart;
        _gapStart = _l2node->_data;
    }

    template<typename charT>
    void Cursor<charT>::_CloseNode()
    {
        _l2node->_length = _gapStart - _l2node->_data;
    }

    int Main(const std::vector<std::u16string>& args);
}