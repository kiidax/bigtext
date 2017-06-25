/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include "linkedlist.h"

#include <string>
#include <cassert>

namespace boar {

    template<typename charT>
    class GapVector
    {
    private:
#if _DEBUG
        static const size_t BlockSize = 100;
#else
        static const size_t BlockSize = 64 * 1024 * 1024;
#endif

    public:
        GapVector() : _ptr(), _gapStart(0), _gapSize(0), _capacity() {}
        ~GapVector()
        {
            delete _ptr;
        }

    public:
        void Reserve(size_t capacity) {
            if (capacity < Size()) capacity = Size();
            capacity = (capacity + BlockSize - 1);
            capacity -= capacity % BlockSize;
            auto newPtr = new charT[capacity];
            if (_ptr != nullptr)
            {
                std::memcpy(newPtr, _ptr, _gapStart);
                size_t copySize = _capacity - (_gapStart + _gapSize);
                std::memcpy(newPtr + capacity - copySize, _ptr + _capacity - copySize, copySize);
                _gapSize += capacity - _capacity;
            }
            else
            {
                assert(_gapStart == 0);
                _gapStart = 0;
                _gapSize = capacity;
            }
            _capacity = capacity;
            delete _ptr;
            _ptr = newPtr;
        }
        size_t Size() const { return _capacity - _gapSize; }
        size_t Capacity() const { return _capacity; }
        charT* Begin() { return _gapStart == 0 ? _ptr + _gapStart + _gapSize : _ptr; }
        charT* End() { return _ptr + _capacity; }
        template<typename IteratorType>
        void Insert(IteratorType start, IteratorType end, size_t pos)
        {
            Reserve(Size() + (end - start));
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
        size_t _capacity;
        size_t _gapStart;
        size_t _gapSize;
        charT* _ptr;
    };
}