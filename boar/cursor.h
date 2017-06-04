/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT>
    class Cursor
    {
    public:
        Cursor(Buffer<charT>& buffer) :
            _buffer(buffer), _node2(&buffer._root), _node1(),
            _node0(), _gapStart(), _gapEnd()
        {
        }
        ~Cursor() {}
        void Append(const charT* start, const charT* end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();

    private:
        void _CloseNode();

    private:
        // This can hold 1k nodes.
        Buffer<charT>& _buffer;
        BufferNode<charT, 2>* _node2;
        BufferNode<charT, 1>* _node1;
        BufferNode<charT, 0>* _node0;
        charT* _gapStart;
        charT* _gapEnd;
    };

    template<typename charT>
    void Cursor<charT>::Append(const charT* start, const charT* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);

        assert(_node2 != nullptr);

        if (_node1 == nullptr)
        {
            assert(_node0 == nullptr);

            _node2->_children.push_back(BufferNode<charT, 1>());
            _node1 = &_node2->_children.back();
        }

        if (_node0 == nullptr)
        {
            assert(_gapStart == nullptr);
            assert(_gapEnd == nullptr);

            (*_node1)._children.push_back(BufferNode<charT, 0>());
            _node0 = &(*_node1)._children.back();
        }

        if (_gapStart == nullptr || _gapEnd - _gapStart < end - start)
        {
            size_t newSize = _node0->_children.size() + (end - start) - (_gapStart == nullptr ? 0 : _gapEnd - _gapStart);
            _node0->_children.resize(newSize);

            // Adjust the pointers.
            _gapStart = &_node0->_children.front();
            _gapEnd = &_node0->_children.back() + 1;
        }

        memcpy(_gapStart, start, (end - start) * sizeof(charT));
        _gapStart += end - start;
    }

    template<typename charT>
    std::basic_string<charT> Cursor<charT>::GetLineAndMoveNext()
    {
        if (_node1 == nullptr)
        {
            return std::basic_string<charT>();
        }

        charT *it;
        for (it = _gapStart; it != &_node0->_children.back() + 1; ++it)
        {
            if (*it == '\n')
                break;
        }

        std::basic_string<charT> ret(_gapStart, it - _gapStart);

        _gapStart = it + 1;
        _gapEnd = _gapEnd;
        if (_gapStart == &_node0->_children.back() + 1)
        {
            _node1 = nullptr;
            _node0 = nullptr;
            _gapStart = nullptr;
            _gapEnd = nullptr;
        }

        return ret;
    }

    template<typename charT>
    void Cursor<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _node2 = &(_buffer._root);
        _node1 = &_node2->_children.front();
        _node0 = &_node1->_children.front();
        _gapStart = &_node0->_children.front();
        _gapEnd = _gapStart;
    }

    template<typename charT>
    void Cursor<charT>::_CloseNode()
    {
        _node0->_children.resize(_gapStart - &_node0->_children.front());
    }
}