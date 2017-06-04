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

    template<typename charT, int level>
    class BufferGap
    {
    public:
        BufferGap() : node(), start(), end() {}
        ~BufferGap() {}

    public:
        typedef BufferNode<charT, level> NodeType;
        NodeType* node;
        int start;
        int end;
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
            _gap0()
        {
        }
        ~Buffer() {}
        void Open(const char16_t * fileName);
        void Insert(const charT* start, const charT* end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();

    private:
        void _CloseNode();

    private:
        BufferNode<charT, 2>* _node2;
        BufferNode<charT, 1>* _node1;
        BufferGap<charT, 0> _gap0;
    };

    template<typename charT>
    void Buffer<charT>::Insert(const charT* start, const charT* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);

        assert(_node2 != nullptr);

        if (_node1 == nullptr)
        {
            assert(_gap0.node == nullptr);

            _node2->_children.push_back(BufferNode<charT, 1>());
            _node1 = &_node2->_children.back();
        }

        if (_gap0.node == nullptr)
        {
            assert(_gap0.start == 0);
            assert(_gap0.end == 0);

            (*_node1)._children.push_back(BufferNode<charT, 0>());
            _gap0.node = &(*_node1)._children.back();
        }

        if (_gap0.end - _gap0.start < end - start)
        {
            size_t newSize = _gap0.node->_children.size() + (end - start) - (_gap0.end - _gap0.start);
            _gap0.node->_children.resize(newSize);

            // Adjust the pointers.
            _gap0.start = 0;
            _gap0.end = newSize;
        }

        memcpy(_gap0.node + _gap0.start, start, (end - start) * sizeof(charT));
        _gap0.start += end - start;
    }

    template<typename charT>
    std::basic_string<charT> Buffer<charT>::GetLineAndMoveNext()
    {
        if (_node1 == nullptr)
        {
            return std::basic_string<charT>();
        }

        charT *it;
        for (it = &_gap0.node->_children.at(_gap0.start); it != &(_gap0.node->_children.back()) + 1; ++it)
        {
            if (*it == '\n')
                break;
        }

        std::basic_string<charT> ret(&_gap0.node->_children.at(_gap0.start), it - _gap0.start);

        _gap0.start = it - &_gap0.node->_children.front() + 1;
        _gap0.end = _gap0.end;
        if (_gap0.start == _gap0.node->_children.size())
        {
            _node1 = nullptr;
            _gap0.node = nullptr;
            _gap0.start = 0;
            _gap0.end = 0;
        }

        return ret;
    }

    template<typename charT>
    void Buffer<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _node2 = &_root;
        _node1 = &_node2->_children.front();
        _gap0.node = &_node1->_children.front();
        _gap0.start = 0;
        _gap0.end = 0;
    }

    template<typename charT>
    void Buffer<charT>::_CloseNode()
    {
        _gap0.node->_children.resize(_gap0.start);
    }
}