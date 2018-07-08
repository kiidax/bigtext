#pragma once

namespace boar 
{
    namespace ast
    {
        enum NodeType 
        {
            SYMBOL,
            NUMBER,
            INTEGER,
            STRING,
            LIST,
        };
        class Node
        {
        public:
            virtual ~Node() {}
            virtual NodeType Type() = 0;
        public:
            Node* next;
        };
        
        class ApplyNode : public Node
        {
        public:
            Node *function;
            Node *arguments;
        };
        
        class SymbolNode : public Node
        {
        public:
            virtual NodeType Type() { return SYMBOL; }
        };
    }
}