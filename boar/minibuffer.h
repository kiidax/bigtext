/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"

#include <cassert>
#include <cstring>

namespace boar {

    template<typename charT>
    class MiniBuffer
    {
    private:
#if _DEBUG
        static const size_t BlockSize = 10;
#else
        static const size_t BlockSize = 64 * 1024 * 1024;
#endif

    public:
        MiniBuffer() : _ptr(), _gapStart(0), _gapSize(0), _capacity() {}
        MiniBuffer(const MiniBuffer& other)
            : _gapStart(other._gapStart),
            _gapSize(other._gapSize),
            _capacity(other._capacity)
        {
            _ptr = new charT[_capacity];
            std::memcpy(_ptr, other._ptr, sizeof(charT) * _capacity);
        }
        ~MiniBuffer()
        {
            delete _ptr;
        }

        size_t GetSize() const { return _capacity - _gapSize; }
        bool IsEmpty() const { return GetSize() == 0; }
        size_t GetCapacity() const { return _capacity; }
        charT& operator [] (size_t position)
        {
            if (position < _gapStart)
            {
                return _ptr[position];
            }
            else
            {
                return _ptr[position + _gapSize];
            }
        }
        operator std::basic_string<charT>()
        {
            std::basic_string<charT> res;
            if (_gapStart > 0)
            {
                res.append(_ptr, _gapStart);
            }
            if (_capacity > _gapStart + _gapSize)
            {
                res.append(_ptr + _gapStart + _gapSize, _capacity - (_gapStart + _gapSize));
            }
            return res;
        }
        void Reserve(size_t capacity)
        {
            if (capacity < GetSize()) capacity = GetSize();
            capacity = (capacity + BlockSize - 1);
            capacity -= capacity % BlockSize;
            if (_capacity != capacity)
            {
                auto newPtr = new charT[capacity];
                if (_ptr != nullptr)
                {
                    std::memcpy(newPtr, _ptr, sizeof(charT) * _gapStart);
                    size_t copySize = _capacity - (_gapStart + _gapSize);
                    std::memcpy(newPtr + capacity - copySize, _ptr + _capacity - copySize, sizeof(charT) * copySize);
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
        }
        void Insert(size_t pos, const charT* s, size_t n)
        {
            Reserve(GetSize() + n);
            assert(pos <= GetSize() && GetSize() + n <= GetCapacity());
            _SetGapPosition(pos);
            assert(n <= _gapSize);
            std::memcpy(_ptr + _gapStart, s, sizeof (charT) * n);
            _gapStart += n;
            _gapSize -= n;
        }
        void Insert(size_t pos, const std::basic_string<charT> str)
        {
            Insert(pos, str.c_str(), str.length());
        }
        void SplitTo(size_t pos, MiniBuffer& other)
        {
            other.Reserve(GetSize() - pos);
            if (pos < _gapStart)
            {
                other._gapStart = _gapStart - pos;
                other._gapSize = other._capacity - (GetSize() - pos);
                std::memcpy(other._ptr, _ptr + pos, sizeof (charT) * (_gapStart - pos));
                std::memcpy(other._ptr + other._gapStart + other._gapSize, _ptr + _gapStart + _gapSize, sizeof (charT) * (_capacity - _gapStart - _gapSize));
                _gapStart = pos;
                _gapSize = _capacity - pos;
            }
            else
            {
                other._gapStart = GetSize() - pos;
                other._gapSize = other._capacity - other._gapStart;
                std::memcpy(other._ptr, _ptr + pos + _gapSize, sizeof(charT) * (GetSize() - pos));
                _gapStart = pos;
                _gapSize = _capacity - pos;
            }
        }

    protected:
        void _SetGapPosition(size_t newGapStart)
        {
            assert(newGapStart < _capacity);
            if (newGapStart > _gapStart)
            {
                std::memmove(_ptr + _gapStart, _ptr + _gapStart + _gapSize, sizeof (charT) * (newGapStart - _gapStart));
            }
            else if (newGapStart < _gapStart)
            {
                std::memmove(_ptr + newGapStart + _gapSize, _ptr + newGapStart, sizeof (charT) * (_gapStart - newGapStart));
            }
            _gapStart = newGapStart;
#ifdef _DEBUG
            std::memset(_ptr + _gapStart, 0xcc, _gapSize);
#endif
        }

    protected:
        size_t _capacity;
        size_t _gapStart;
        size_t _gapSize;
        charT* _ptr;
    };
}