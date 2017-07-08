/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include "minibuffer.h"
#include "linkedlist.h"

#include <string>
#include <assert.h>

namespace boar {

    template<typename charT>
    class BufferNode : public MiniBuffer<charT>
    {

    };

    template<typename charT>
    class Buffer
    {
    public:
        Buffer() : _lineSeparator('\n')
        {
            _currentNode = _nodeList.begin();
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
        void Insert(IteratorType first, IteratorType last)
        {
            size_t size = last - first;
            if (size > _currentNode->Capacity() - _currentNode->Size())
            {
                _currentNode = _currentNode.GetCurrent()->Split();
            }
            _currentNode->Insert<IteratorType>(first, last, _currentNode->Size());
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