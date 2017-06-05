/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include <boar/base.h>

namespace boar {

    template<typename charT> class Buffer;
    template<typename charT> class BufferGap;

    const int BufferLevel = 2;

    template<typename ChildNodeType>
    class BufferNodeBase {
    public:
        int numLines;
        BufferNodeBase() : numLines() {}
        ~BufferNodeBase() {}
        ChildNodeType& operator [](size_t n) { return _children[n]; }
        typename std::vector<ChildNodeType>::iterator Begin() { return _children.begin(); }
        typename std::vector<ChildNodeType>::iterator End() { return _children.end(); }
        size_t Size() const { return _children.size(); }
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
    };

    template<typename charT>
    class BufferNode<charT, 0> : public BufferNodeBase<charT>
    {
    };

    template<typename BufferNodeType>
    class BufferGap
    {
    public:
        BufferGap() : _node(), _start(), _end() {}
        BufferGap(BufferNodeType* node, size_t start=0, size_t end=0) : _node(node), _start(start), _end(end)
        {
            assert(IsArgInRange(start, end));
        }
        ~BufferGap() {}
        operator bool() const { return _node != nullptr; }
        bool IsArgInRange(size_t start, size_t end) const {
            return start < INT_MAX && end < INT_MAX && start <= end;
        }
        BufferGap& operator = (BufferNodeType* node) {
            _node = node;
            _start = 0;
            _end = 0;
            return *this;
        }
        // Reserve the size of the gap. Do nothing if
        // the current gap size is larger than the given size.
        void Reserve(size_t size) 
        {
            if (_end - _start < size)
            {
                _node->ReserveGap(_start, _end, size);
                _end = _start + size;
            }
        }
        void Unreserve()
        {
            _node->ReserveGap(_start, _end, 0);
            _start = 0;
            _end = 0;
        }
        template<typename IteratorType>
        void CopyToGap(IteratorType first, IteratorType last)
        {
            std::copy(first, last, _node->Begin() + _start);
            _start += last - first;
        }

    public:
        BufferNodeType* _node;
        size_t _start;
        size_t _end;
    };

    template<typename charT>
    class Buffer
    {
        typedef BufferNode<charT, BufferLevel> ChildNodeType;
    private:
        ChildNodeType _root;
    public:
        Buffer() : _gap2(&_root), _gap1(), _gap0()
        {
            _lineSeparator = '\n';
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
        const unsigned MaxNode0Size = 200;
        const unsigned MaxNode1Size = 5;
#else
        const int MaxNode0Size = 1024;
        const int MaxNode1Size = 5;
#endif
        charT _lineSeparator;
        BufferGap<BufferNode<charT, 2>> _gap2;
        BufferGap<BufferNode<charT, 1>> _gap1;
        BufferGap<BufferNode<charT, 0>> _gap0;
    };

    template<typename charT>
    template<typename charIterator>
    void Buffer<charT>::Insert(charIterator first, charIterator last)
    {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(first < last);

        // This is the root and always there.
        assert(static_cast<bool>(_gap2));
        assert(_gap2._node == &_root);

        if (!_gap1)
        {
            assert(!_gap0);
            _gap2.Reserve(2);
            _gap1 = &(*_gap2._node)[_gap2._start];
            _gap2._start++;
        }

        if (!_gap0)
        {
            _gap1.Reserve(2);
            _gap0 = &(*_gap1._node)[_gap1._start];
            _gap1._start++;
        }

        assert(_gap2);
        assert(_gap1);
        assert(_gap0);

        if (_gap0._start + (last - first) > MaxNode0Size)
        {
            // The data before the L0 gap and the inserted data won't fit in the node,
            // so make a new L1 node.
            if (_gap1._start + 1 > MaxNode1Size)
            {
                // The data before the L1 gap won't fit, so make a new L2 node.

                // _gap1.node must always points to the one node before _gap2.start.
                assert(_gap1._node == &(*_gap2._node)[_gap2._start - 1]);

                int oldGapStart = _gap1._start;
                int oldGapEnd = _gap1._end;
                _gap1.Unreserve();

                if (_gap1._end < _gap1._node->Size())
                {
                    _gap2.Reserve(2);
                    auto oldNode = &(*_gap2._node)[_gap2._start - 1];
                }
                else
                {
                    _gap2.Reserve(1);
                    auto oldNode = &(*_gap2._node)[_gap2._start - 1];
                }
                _gap1._node = &(*_gap2._node)[_gap2._start++];
                _gap1._start = 0;
                _gap1._end = 0;
            }
            _gap0.Reserve(0);
            _gap1.Reserve(1);
            _gap0._node = &(*_gap1._node)[_gap1._start++];
            _gap0._start = 0;
            _gap0._end = 0;
        }

        _gap0.Reserve(last - first);
        _gap0.CopyToGap(first, last);
    }

    template<typename charT>
    std::basic_string<charT> Buffer<charT>::GetLineAndMoveNext()
    {
        if (_gap1._node == nullptr)
        {
            return std::basic_string<charT>();
        }

        typename std::vector<charT>::iterator it;
        for (it = _gap0._node->Begin() + _gap0._start; it != _gap0._node->End(); ++it)
        {
            if (*it == _lineSeparator)
                break;
        }

        std::basic_string<charT> ret(_gap0._node->Begin() + _gap0._start, it);

        _gap0._start = it - _gap0._node->Begin() + 1;
        _gap0._end = _gap0._start;
        if (_gap0._start == _gap0._node->Size())
        {
            _gap1._node = nullptr;
            _gap0._node = nullptr;
            _gap0._start = 0;
            _gap0._end = 0;
        }

        return ret;
    }

    template<typename charT>
    void Buffer<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _gap2._node = &_root;
        _gap1._node = &(*_gap2._node)[0];
        _gap0._node = &(*_gap1._node)[0];
        _gap0._start = 0;
        _gap0._end = 0;
    }

    template<typename charT>
    void Buffer<charT>::_CloseNode()
    {
        _gap0.Unreserve();
    }
}