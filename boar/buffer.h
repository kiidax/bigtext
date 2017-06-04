/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT> class Cursor;

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
        friend class Cursor<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class BufferNode<charT, 0> : BufferNodeBase
    {
        typedef charT ChildNodeType;
        friend class Cursor<charT>;
    private:
        std::vector<ChildNodeType> _children;
    };

    template<typename charT>
    class Buffer
    {
        typedef BufferNode<charT, BufferLevel> ChildNodeType;
        friend class Cursor<charT>;
    private:
        ChildNodeType _root;
    public:
        Buffer() {}
        ~Buffer() {}
    };

    template<typename charT> class Cursor;

    template<typename charT>
    class Cursor
    {
    public:
        Cursor(Buffer<charT>& buffer) :
            _buffer(buffer), _l0node(&buffer._root), _l1node(),
            _l2node(), _gapStart(), _gapEnd()
        {
        }
        ~Cursor() {}
        void Open(const char16_t * fileName);
        void Append(const charT* start, const charT* end);
        std::basic_string<charT> GetLineAndMoveNext();
        void MoveBeginningOfBuffer();

    private:
        void _CloseNode();

    private:
        // This can hold 1k nodes.
        Buffer<charT>& _buffer;
        BufferNode<charT, 2>* _l0node;
        BufferNode<charT, 1>* _l1node;
        BufferNode<charT, 0>* _l2node;
        charT* _gapStart;
        charT* _gapEnd;
    };

    template<typename charT>
    void Cursor<charT>::Append(const charT* start, const charT* end)
    {
        assert(start != nullptr);
        assert(end != nullptr);

        assert(_l0node != nullptr);

        if (_l1node == nullptr)
        {
            assert(_l2node == nullptr);

            _l0node->_children.push_back(BufferNode<charT, 1>());
            _l1node = &_l0node->_children.back();
        }

        if (_l2node == nullptr)
        {
            assert(_gapStart == nullptr);
            assert(_gapEnd == nullptr);

            (*_l1node)._children.push_back(BufferNode<charT, 0>());
            _l2node = &(*_l1node)._children.back();
        }

        if (_gapStart == nullptr || _gapEnd - _gapStart < end - start)
        {
            size_t newSize = _l2node->_children.size() + (end - start) - (_gapStart == nullptr ? 0 : _gapEnd - _gapStart);
            _l2node->_children.resize(newSize);

            // Adjust the pointers.
            _gapStart = &_l2node->_children.front();
            _gapEnd = &_l2node->_children.back() + 1;
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
        for (it = _gapStart; it != &_l2node->_children.back() + 1; ++it)
        {
            if (*it == '\n')
                break;
        }

        std::basic_string<charT> ret(_gapStart, it - _gapStart);

        _gapStart = it + 1;
        _gapEnd = _gapEnd;
        if (_gapStart == &_l2node->_children.back() + 1)
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
        _l0node = &(_buffer._root);
        _l1node = &_l0node->_children.front();
        _l2node = &_l1node->_children.front();
        _gapStart = &_l2node->_children.front();
        _gapEnd = _gapStart;
    }

    template<typename charT>
    void Cursor<charT>::_CloseNode()
    {
        _l2node->_children.resize(_gapStart - &_l2node->_children.front());
    }
}