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
        typedef BufferNode<charT, level> NodeType;
    public:
        BufferGap() : node(), start(), end() {}
        ~BufferGap() {}
        bool IsEntered() { return node != nullptr; }
        void Enter(NodeType* newNode, int newPos)
        {
            assert(!IsEntered());
            node = newNode;
            start = newPos;
            end = newPos;
        }
        void Leave()
        {
            assert(IsEntered());
            //assert(node->_children.size() == end); // TODO
            node->ReserveGap(start, end, 0);
            node = nullptr;
#ifdef _DEBUG
            start = 0xdeadbeef;
            end = 0xbeefdead;
#endif
        }
        // Reserve the size of the gap. Do nothing if
        // the current gap size is larger than the given size.
        void Reserve(int size) 
        {
            if (end - start < size)
            {
                node->ReserveGap(start, end, size);
                end = start + size;
            }
        }

    public:
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
        Buffer() : _gap2(), _gap1(), _gap0()
        {
            _lineSeparator = '\n';
            _gap2.Enter(&_root, 0);
        }
        ~Buffer() {}
        void Open(const char16_t* fileName);
        template<typename charIterator>
        void Insert(charIterator start, charIterator end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();
#ifdef _DEBUG
        void Dump();
#endif

    private:
        void _CloseNode();

    private:
#ifdef _DEBUG
        const int MaxNode0Size = 200;
        const int MaxNode1Size = 5;
#else
        const int MaxNode0Size = 1024;
        const int MaxNode1Size = 5;
#endif
        charT _lineSeparator;
        BufferGap<charT, 2> _gap2;
        BufferGap<charT, 1> _gap1;
        BufferGap<charT, 0> _gap0;
    };

    template<typename charT>
    template<typename charIterator>
    void Buffer<charT>::Insert(charIterator first, charIterator last)
    {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(first < last);

        // This is the root and always there.
        assert(_gap2.IsEntered());
        assert(_gap2.node == &_root);

        if (!_gap1.IsEntered())
        {
            assert(_gap0.node == nullptr);
            _gap2.Reserve(10);
            _gap1.Enter(&_gap2.node->_children.at(_gap2.start), 0);
            _gap2.start++;
        }

        if (!_gap0.IsEntered())
        {
            assert(_gap0.start == 0);
            assert(_gap0.end == 0);
            _gap1.Reserve(10);
            _gap0.Enter(&_gap1.node->_children.at(_gap1.start), 0);
            _gap1.start++;
        }

        assert(_gap2.IsEntered());
        assert(_gap1.IsEntered());
        assert(_gap0.IsEntered());

        if (_gap0.start + (last - first) > MaxNode0Size)
        {
            // The data before the L0 gap and the inserted data won't fit in the node,
            // So make a new node.
            if (_gap1.start + 1 > MaxNode1Size)
            {
                // The data before the L1 gap won't fit.

                // _gap1.node must always points to the one node before _gap2.start.
                assert(_gap1.node == &_gap2.node->_children[_gap2.start - 1]);
                _gap1.node = nullptr;
                _gap2.Reserve(1);
                _gap1.node = &_gap2.node->_children[_gap2.start++];
                _gap1.start = 0;
                _gap1.end = 0;
            }
            _gap0.Reserve(0);
            _gap1.Reserve(1);
            _gap0.node = &_gap1.node->_children[_gap1.start++];
            _gap0.start = 0;
            _gap0.end = 0;
        }

        _gap0.Reserve(last - first);
        std::copy(first, last, _gap0.node->_children.begin() + _gap0.start);
        _gap0.start += last - first;
    }

    template<typename charT>
    std::basic_string<charT> Buffer<charT>::GetLineAndMoveNext()
    {
        if (_gap1.node == nullptr)
        {
            return std::basic_string<charT>();
        }

        charT *it;
        for (it = &_gap0.node->_children.at(_gap0.start); it != &(_gap0.node->_children.back()) + 1; ++it)
        {
            if (*it == _lineSeparator)
                break;
        }

        std::basic_string<charT> ret(&_gap0.node->_children.at(_gap0.start), it);

        _gap0.start = it - &_gap0.node->_children.front() + 1;
        _gap0.end = _gap0.start;
        if (_gap0.start == _gap0.node->_children.size())
        {
            _gap1.node = nullptr;
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
        _gap2.node = &_root;
        _gap1.node = &_gap2.node->_children.front();
        _gap0.node = &_gap1.node->_children.front();
        _gap0.start = 0;
        _gap0.end = 0;
    }

    template<typename charT>
    void Buffer<charT>::_CloseNode()
    {
        _gap0.node->_children.resize(_gap0.start);
    }
}