/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>
#include <string>
#include <assert.h>

namespace boar {

    template<typename charT> class Buffer;

    template<typename T>
    class GapVector
    {
    public:
#if _DEBUG
        static const size_t MinCapacity = 400;
#else
        static const size_t MinCapacity = 64 * 1024 * 1024;
#endif

    protected:
        GapVector() : _numLines() {
            _capacity = MinCapacity;
            _gapStart = 0;
            _gapSize = _capacity;
            _ptr = new T[_capacity];
        }
        ~GapVector()
        {
            delete _ptr;
        }

    public:
        size_t Size() const { return _capacity - _gapSize; }
        size_t Capacity() const { return _capacity; }
        T* Begin() { return _gapStart == 0 ? _ptr + _gapStart + _gapSize : _ptr; }
        T* End() { return _ptr + _capacity; }
        template<typename IteratorType>
        void Insert(IteratorType start, IteratorType end, size_t pos)
        {
            assert(pos <= Size() && Size() + (end - start) <= Capacity());
            if (pos != _gapStart)
            {
                _MoveGapStart(pos);
            }
            T* p = _ptr + _gapStart;
            size_t c = 0;
            for (auto it = start; it != end; ++it)
            {
                *(p++) = *it;
                ++c;
            }
            _gapStart += c;
            _gapSize -= c;
        }

    protected:
        void _MoveGapStart(size_t newGapStart)
        {
            assert(newGapStart < _capacity);
            if (newGapStart > _gapStart)
            {
                memmove(_ptr + _gapStart, _ptr + _gapStart + _gapSize, (newGapStart - _gapStart));
            }
            else
            {
                memmove(_ptr + newGapStart, _ptr + newGapStart + _gapSize, (_gapStart - newGapStart));
            }
        }

    protected:
        int _numLines;
        size_t _capacity;
        size_t _gapStart;
        size_t _gapSize;
        T* _ptr;
    };

    template<typename charT>
    class BufferNode : public GapVector<charT>
    {
    public:
        BufferNode() : _prev(this), _next(this) {}
        ~BufferNode() {}
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