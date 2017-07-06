/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"

#include <cassert>
#include <cstring>

namespace boar {

    template<typename charT>
    class GapVector
    {
    private:
        class Iterator : public std::iterator<std::input_iterator_tag, charT>
        {
        private:
            GapVector& _vector;
            size_t _position;

        public:
            Iterator(GapVector& vector, size_t position)
                : _vector(vector), _position(position) {}
            bool operator ==(const Iterator& other) const
            {
                assert(&_vector == &other._vector);
                return _position == other._position;
            }
            bool operator !=(const Iterator& other) const
            {
                return !(*this == other);
            }
            Iterator& operator ++()
            {
                ++_position;
                return *this;
            }
            charT& operator *()
            {
                return _vector[_position];
            }
        };

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
        void Reserve(size_t capacity)
        {
            if (capacity < size()) capacity = size();
            capacity = (capacity + BlockSize - 1);
            capacity -= capacity % BlockSize;
            if (_capacity != capacity)
            {
                auto newPtr = new charT[capacity];
                if (_ptr != nullptr)
                {
                    std::memcpy(newPtr, _ptr, sizeof (charT) * _gapStart);
                    size_t copySize = _capacity - (_gapStart + _gapSize);
                    std::memcpy(newPtr + capacity - copySize, _ptr + _capacity - copySize, sizeof (charT) * copySize);
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
        size_t size() const { return _capacity - _gapSize; }
        size_t Capacity() const { return _capacity; }
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
        Iterator begin() { return Iterator(*this, 0); }
        Iterator end() { return Iterator(*this, size()); }
        template<typename IteratorType>
        void Insert(IteratorType start, IteratorType end, size_t pos)
        {
            Reserve(size() + (end - start));
            assert(pos <= size() && size() + (end - start) <= Capacity());
            _SetGapStart(pos);
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
        void SplitInto(size_t pos, GapVector& other)
        {
            if (pos < _gapStart)
            {
                other.Reserve(size() - pos);
                other._gapStart = _gapStart - pos;
                other._gapSize = other._capacity - (size() - pos);
                std::memcpy(other._ptr, _ptr + pos, sizeof (charT) * _gapStart - pos);
                std::memcpy(other._ptr + other._gapStart + other._gapSize, _ptr + _gapStart + _gapSize, sizeof (charT) * (_capacity - _gapStart - _gapSize));
                _gapStart = pos;
                _gapSize = _capacity - pos;
            }
            else
            {
                assert(false);
            }
        }

    protected:
        void _SetGapStart(size_t newGapStart)
        {
            assert(newGapStart < _capacity);
            if (newGapStart > _gapStart)
            {
                memmove(_ptr + _gapStart, _ptr + _gapStart + _gapSize, sizeof (charT) * (newGapStart - _gapStart));
            }
            else if (newGapStart < _gapStart)
            {
                memmove(_ptr + newGapStart + _gapSize, _ptr + newGapStart, sizeof (charT) * (_gapStart - newGapStart));
            }
            _gapStart = newGapStart;
        }

    protected:
        size_t _capacity;
        size_t _gapStart;
        size_t _gapSize;
        charT* _ptr;
    };
}