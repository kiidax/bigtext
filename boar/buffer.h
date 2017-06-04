/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT> class Buffer;

    const int BufferLevel = 2;

    template<typename ChildNodeType>
    class BufferNodeBase {
    public:
        int numLines;
        BufferNodeBase() : numLines() {}
        ~BufferNodeBase() {}
        void ReserveGap(int currentGapStart, int currentGapEnd, int newGapSize)
        {
            assert(_children.size() == currentGapEnd); // TODO
            int currentGapSize = currentGapEnd - currentGapStart;
            int newSize = _children.size() + (newGapSize - currentGapSize);
            _children.resize(newSize);
        }
    protected:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT, int level>
    class BufferNode : public BufferNodeBase<BufferNode<charT, level - 1>>
    {
        friend class Buffer<charT>;
    };

    template<typename charT>
    class BufferNode<charT, 0> : public BufferNodeBase<charT>
    {
        friend class Buffer<charT>;
    };

    template<typename charT, int level>
    class BufferGap
    {
    public:
        BufferGap() : node(), start(), end() {}
        ~BufferGap() {}
        void Reserve(int size) 
        {
            if (end - start < size)
            {
                node->ReserveGap(start, end, size);
                end = start + size;
            }
        }

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

        // This is the root and always there.
        assert(_node2 != &_root);

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

        _gap0.Reserve(end - start);
        memcpy(&_gap0.node->_children.front() + _gap0.start, start, (end - start) * sizeof(charT));
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