/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include "gapvector.h"
#include "linkedlist.h"

#include <string>
#include <assert.h>

namespace boar {

    template<typename charT>
    class BufferNode : public GapVector<charT>
    {

    };

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
        LinkedList<BufferNode<charT>> _nodeList;
        LinkedListIterator<BufferNode<charT>> _currentNode;
    };
}