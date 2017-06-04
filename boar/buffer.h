/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT> class Buffer;

    const int BufferLevel = 2;

    class BufferNodeBase {
    public:
        int numLines;
        BufferNodeBase() : numLines() {}
        ~BufferNodeBase() {}
    };

    template<typename charT, int level>
    class BufferNode : BufferNodeBase
    {
        typedef BufferNode<charT, level - 1> ChildNodeType;
        friend class Buffer<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class BufferNode<charT, 0> : BufferNodeBase
    {
        typedef charT ChildNodeType;
        friend class Buffer<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class Buffer
    {
        typedef BufferNode<charT, BufferLevel> ChildNodeType;
        friend class Buffer<charT>;
    private:
        ChildNodeType _root;
    public:
        Buffer() :
            _node2(&_root), _node1(),
            _node0(), _gapStart(), _gapEnd()
        {
        }
        ~Buffer() {}
        void Open(const char16_t * fileName);
        void Append(const charT* start, const charT* end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();

    private:
        void _CloseNode();

    private:
        BufferNode<charT, 2>* _node2;
        BufferNode<charT, 1>* _node1;
        BufferNode<charT, 0>* _node0;
        charT* _gapStart;
        charT* _gapEnd;
    };

    template<typename charT>
    void Buffer<charT>::Append(const charT* start, const charT* end)
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
    std::basic_string<charT> Buffer<charT>::GetLineAndMoveNext()
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
    void Buffer<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _node2 = &_root;
        _node1 = &_node2->_children.front();
        _node0 = &_node1->_children.front();
        _gapStart = &_node0->_children.front();
        _gapEnd = _gapStart;
    }

    template<typename charT>
    void Buffer<charT>::_CloseNode()
    {
        _node0->_children.resize(_gapStart - &_node0->_children.front());
    }
}