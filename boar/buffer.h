/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>
#include <string>
#include <assert.h>

namespace boar {

    template<typename charT> class Buffer;

    class BufferNodeBase
    {
    public:
#if _DEBUG
        static const size_t MinCapacity = 400;
#else
        static const size_t MinCapacity = 64 * 1024 * 1024;
#endif

    protected:
        BufferNodeBase() : _numLines() {}

    public:
        size_t Size() const { return _capacity - (_gapEnd - _gapStart); }
        size_t Capacity() const { return _capacity; }

    protected:
        int _numLines;
        size_t _capacity;
        size_t _gapStart;
        size_t _gapEnd;
    };

    template<typename charT>
    class BufferNode : public BufferNodeBase
    {
    public:
        BufferNode() : _prev(this), _next(this)
        {
            _capacity = MinCapacity;
            _gapStart = 0;
            _gapEnd = _capacity;
            _ptr = new charT[_capacity];
        }
        ~BufferNode()
        {
            delete _ptr;
        }
        charT* Begin() { return _ptr; }
        charT* End() { return _ptr + _gapStart; }
        void Insert(size_t pos, void* data, size_t size)
        {
            assert(pos < _capacity && pos + size < _capacity);
            memcpy(_ptr + pos, data, size);
            _gapStart += size;
        }
        template<typename IteratorType>
        void Insert(IteratorType start, IteratorType end, size_t pos)
        {
            assert(pos < _capacity && pos + (end - start) < _capacity);
            charT* p = _ptr + pos;
            for (auto it = start; it != end; ++it)
            {
                *(p++) = *it;
            }
            _gapStart += end - start;
        }
        BufferNode<charT>* Split()
        {
            auto newNode = new BufferNode<charT>();
            newNode->_prev = this;
            newNode->_next = this->_next;
            _next = newNode;
            newNode->_next->_prev = newNode;
            return newNode;
        }
        BufferNode<charT>* Next() { return _next; }

    protected:
        BufferNode<charT>* _prev;
        BufferNode<charT>* _next;
        charT* _ptr;
    };

    class BufferBase
    {
    public:
        BufferBase() {}
        ~BufferBase() {}
    protected:
    };

    template<typename charT>
    class Buffer : BufferBase
    {
    public:
        Buffer() : _lineSeparator('\n')
        {
            _startNode = _endNode = _currentNode = new BufferNode<charT>();
        }
        ~Buffer()
        {
            delete _startNode;
        }
        void Open(const char16_t* fileName);
        void Dump();
        void Insert(const charT* data, size_t size)
        {
            if (size > _currentNode->Capacity() - _currentNode->Size())
            {
                _currentNode = _currentNode->Split();
            }
            _currentNode->Insert(data, size);
        }
        template<typename IteratorType>
        void Insert(IteratorType begin, IteratorType end)
        {
            size_t size = end - begin;
            if (size > _currentNode->Capacity() - _currentNode->Size())
            {
                _currentNode = _currentNode->Split();
            }
            _currentNode->Insert<IteratorType>(begin, end, _currentNode->Size());
        }
        void Test() {}
        void MoveBeginningOfBuffer() {}
        std::basic_string<charT> GetLineAndMoveNext() { return std::basic_string<charT>(); }
    protected:
        charT _lineSeparator;
        BufferNode<charT>* _startNode;
        BufferNode<charT>* _endNode;
        BufferNode<charT>* _currentNode;
    };
}