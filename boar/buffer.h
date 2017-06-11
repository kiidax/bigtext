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
        typedef typename std::vector<ChildNodeType>::iterator IteratorType;

    public:
        BufferNodeBase() : _numLines() {}
        ~BufferNodeBase() {}
        ChildNodeType& operator [](size_t n) { return _children[n]; }
        typename IteratorType Begin() { return _children.begin(); }
        typename IteratorType End() { return _children.end(); }
        size_t Size() const { return _children.size(); }
        void ReserveGap(int currentGapStart, int currentGapEnd, int newGapSize)
        {
            assert(_children.size() == currentGapEnd); // TODO
            int currentGapSize = currentGapEnd - currentGapStart;
            int newSize = _children.size() + (newGapSize - currentGapSize);
            _children.resize(newSize);
        }

    protected:
        int _numLines;
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
        typedef typename BufferNodeType::IteratorType IteratorType;

    public:
        BufferGap() : _startNode(), _startIndex(), _endIndex() {}
        BufferGap(BufferNodeType* node, size_t start=0, size_t end=0) : _startNode(node), _startIndex(start), _endIndex(end)
        {
            assert(IsArgInRange(start, end));
        }
        ~BufferGap() {}
        operator bool() const { return _startNode != nullptr; }
        bool IsArgInRange(size_t start, size_t end) const {
            return start < INT_MAX && end < INT_MAX && start <= end;
        }
        BufferGap& operator = (BufferNodeType* node) {
            _startNode = node;
            _startIndex = 0;
            _endIndex = 0;
            return *this;
        }
        size_t GapSize() const { return _endIndex - _startIndex; }
        IteratorType BeginGap() { return _startNode->Begin() + _startIndex; }
        IteratorType EndGap() { return _startNode->Begin() + _endIndex; }
        // Reserve the size of the gap. Do nothing if
        // the current gap size is larger than the given size.
        void Reserve(size_t size) 
        {
            if (_endIndex - _startIndex < size)
            {
                _startNode->ReserveGap(_startIndex, _endIndex, size);
                _endIndex = _startIndex + size;
            }
        }
        void Unreserve()
        {
            _startNode->ReserveGap(_startIndex, _endIndex, 0);
            _startIndex = 0;
            _endIndex = 0;
        }
        template<typename IteratorType>
        void CopyToGap(IteratorType first, IteratorType last)
        {
            std::copy(first, last, _startNode->Begin() + _startIndex);
            _startIndex += last - first;
        }

    private:
        BufferNodeType* _startNode;
        BufferNodeType* _endNode;
        size_t _startIndex;
        size_t _endIndex;

        template<typename charT>
        friend class Buffer;
    };

    template<typename charT>
    class Buffer
    {
        typedef BufferNode<charT, BufferLevel> ChildNodeType;
    private:
        const static int MaxNodeLevel = 2;
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
        void Test() {
            BufferNode<charT, 0>* node = _AllocNodeToEnd<0>();
            node = _AllocNodeToStart<0>();
        }

    private:
        template<int level>
        BufferGap<BufferNode<charT, level>>* _GetBufferGap();
        template<>
        BufferGap<BufferNode<charT, 0>>* _GetBufferGap<0>() { return &_gap0; }
        template<>
        BufferGap<BufferNode<charT, 1>>* _GetBufferGap<1>() { return &_gap1; }
        template<>
        BufferGap<BufferNode<charT, 2>>* _GetBufferGap<2>() { return &_gap2; }

        template<int level>
        size_t _GetMaxNodeSize() const;
        template<>
        size_t _GetMaxNodeSize<0>() const { return 10; }
        template<>
        size_t _GetMaxNodeSize<1>() const { return 3; }
        template<>
        size_t _GetMaxNodeSize<2>() const { return 100; }

        template<int level>
        BufferNode<charT, level>* _AllocNodeToStart()
        {
            BufferGap<BufferNode<charT, level + 1>>* gap = _GetBufferGap<level + 1>();
            if (gap->_startNode == gap->_endNode)
            {
                if (gap->_endIndex - gap->_startIndex >= 2)
                {
                    return &(*gap->_endNode)[--gap->_endIndex];
                }
            }
            else
            {
                if (gap->_endNode->Size() >= _GetMaxNodeSize<level + 1>() && gap->_endIndex == 0)
                {
                    gap->_endNode = _AllocNodeToEnd<level + 1>();
                    //gap->_endNode->Reserve(10);
                    gap->_endIndex = 10;
                }
            }
            return nullptr;
        }

        template<>
        BufferNode<charT, MaxNodeLevel>* _AllocNodeToStart<MaxNodeLevel>()
        {
            return nullptr;
        }

        template<int level>
        BufferNode<charT, level>* _AllocNodeToEnd()
        {
            BufferGap<BufferNode<charT, level + 1>>* gap = _GetBufferGap<level + 1>();
            if (gap->_startNode == gap->_endNode)
            {
                if (gap->_endIndex - gap->_startIndex >= 2)
                {
                    return &(*gap->_endNode)[--gap->_endIndex];
                }
            }
            else
            {
                if (gap->_endNode->Size() >= _GetMaxNodeSize<level + 1>() && gap->_endIndex == 0)
                {
                    gap->_endNode = _AllocNodeToEnd<level + 1>();
                    //gap->_endNode->Reserve(10);
                    gap->_endIndex = 10;
                }
            }
            return nullptr;
        }

        template<>
        BufferNode<charT, MaxNodeLevel>* _AllocNodeToEnd<MaxNodeLevel>()
        {
            return nullptr;
        }

        template<int level>
        void _AllocNodeToEnd2(typename BufferGap<BufferNode<charT, level>>& gap,
            bool toEnd,
            typename BufferNode<charT, level>::ChildNodeType& newNode1,
            typename BufferNode<charT, level>::ChildNodeType& newNode2)
        {
            gap.Reserve(2);
            newNode1 = &gap[gap._startIndex];
            if (toEnd)
            {
                newNode2 = &gap[gap._endIndex - 1];
            }
            else
            {
                newNode2 = &gap[gap._startIndex + 1];
            }
        }
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
        assert(_gap2._startNode == &_root);

        if (!_gap1)
        {
            assert(!_gap0);
            _gap2.Reserve(1);
            _gap1 = &*_gap2.BeginGap();
        }

        if (!_gap0)
        {
            _gap1.Reserve(1);
            _gap0 = &*_gap1.BeginGap();
        }

        assert(_gap2);
        assert(_gap1);
        assert(_gap0);

        if (_gap0._startIndex + (last - first) > MaxNode0Size)
        {
            ++_gap1._startIndex;
            // The data before the L0 gap and the inserted data won't fit in the node,
            // so make a new L1 node.
            if (_gap1._startIndex + 1 > MaxNode1Size)
            {
                // The data before the L1 gap won't fit, so make a new L2 node.

                // _gap1.node must always points to the one node before _gap2.start.
                assert(_gap1._startNode == &(*_gap2._startNode)[_gap2._startIndex]);

                int oldGapStart = _gap1._startIndex;
                int oldGapEnd = _gap1._endIndex;
                _gap1.Unreserve();

                if (_gap1._endIndex < _gap1._startNode->Size())
                {
                    _gap2.Reserve(2);
                    auto oldNode = &(*_gap2._startNode)[_gap2._startIndex];
                }
                else
                {
                    _gap2.Reserve(1);
                    auto oldNode = &(*_gap2._startNode)[_gap2._startIndex];
                }
                _gap1._startNode = &(*_gap2._startNode)[++_gap2._startIndex];
                _gap1._startIndex = 0;
                _gap1._endIndex = 0;
            }
            _gap0.Reserve(0);
            _gap1.Reserve(2);
            _gap0._startNode = &(*_gap1._startNode)[_gap1._startIndex];
            _gap0._startIndex = 0;
            _gap0._endIndex = 0;
        }

        _gap0.Reserve(last - first);
        _gap0.CopyToGap(first, last);
    }

    template<typename charT>
    std::basic_string<charT> Buffer<charT>::GetLineAndMoveNext()
    {
        if (!_gap1)
        {
            return std::basic_string<charT>();
        }

        typename std::vector<charT>::iterator it;
        for (it = _gap0.BeginGap(); it != _gap0.EndGap(); ++it)
        {
            if (*it == _lineSeparator)
                break;
        }

        std::basic_string<charT> ret(_gap0.BeginGap(), it);

        _gap0._startIndex = it - _gap0._startNode->Begin() + 1;
        _gap0._endIndex = _gap0._startIndex;
        if (_gap0._startIndex == _gap0._startNode->Size())
        {
            _gap1._startNode = nullptr;
            _gap0._startNode = nullptr;
            _gap0._startIndex = 0;
            _gap0._endIndex = 0;
        }

        return ret;
    }

    template<typename charT>
    void Buffer<charT>::MoveBeginningOfBuffer()
    {
        _CloseNode();
        _gap2._startNode = &_root;
        _gap1._startNode = &(*_gap2._startNode)[0];
        _gap0._startNode = &(*_gap1._startNode)[0];
        _gap0._startIndex = 0;
        _gap0._endIndex = 0;
    }

    template<typename charT>
    void Buffer<charT>::_CloseNode()
    {
        _gap0.Unreserve();
    }
}