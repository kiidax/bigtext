/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include "linkedlist.h"

#include <string>
#include <assert.h>

namespace boar {

    template<typename charT> class Buffer;

    namespace
    {
        template<typename charT>
        class GapVector
        {
        public:
#if _DEBUG
            static const size_t MinCapacity = 400;
#else
            static const size_t MinCapacity = 64 * 1024 * 1024;
#endif

        public:
            GapVector() : _numLines() {
                _capacity = MinCapacity;
                _gapStart = 0;
                _gapSize = _capacity;
                _ptr = new charT[_capacity];
            }
            ~GapVector()
            {
                delete _ptr;
            }

        public:
            size_t Size() const { return _capacity - _gapSize; }
            size_t Capacity() const { return _capacity; }
            charT* Begin() { return _gapStart == 0 ? _ptr + _gapStart + _gapSize : _ptr; }
            charT* End() { return _ptr + _capacity; }
            template<typename IteratorType>
            void Insert(IteratorType start, IteratorType end, size_t pos)
            {
                assert(pos <= Size() && Size() + (end - start) <= Capacity());
                if (pos != _gapStart)
                {
                    _MoveGapStart(pos);
                }
                charT* p = _ptr + _gapStart;
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
            charT* _ptr;
        };
    }

    template<typename charT>
    class Buffer
    {
    public:
        Buffer() : _lineSeparator('\n')
        {
            _currentNode = _nodeList.Begin();
        }
        ~Buffer()
        {
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
                _currentNode = _currentNode.Get()->Split();
            }
            _currentNode->Insert<IteratorType>(begin, end, _currentNode->Size());
        }
        void Test() {}
        void MoveBeginningOfBuffer() {}
        std::basic_string<charT> GetLineAndMoveNext() { return std::basic_string<charT>(); }
    protected:
        charT _lineSeparator;
        LinkedList<GapVector<charT>> _nodeList;
        LinkedListIterator<GapVector<charT>> _currentNode;
    };
}