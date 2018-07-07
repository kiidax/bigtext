/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include <string>

namespace boar {
    class Lexer
    {
    public:
        typedef wchar_t CharType;
        typedef std::basic_string<CharType> StringType;
        enum TokenType
        {
            INVALID,
            END,
            INTEGER,
            NUMBER,
            STRING,
            OPERATOR,
            IDENT
        };
        union TokenValue
        {
            long integer;
            double number;
            const CharType *string;
        };
    public:
        Lexer(const CharType *start, const CharType *end);
        TokenType Lex();
        StringType String() { return StringType(_value.string, _current); }
        double Number() { return _value.number; }
        double Integer() { return _value.integer; }
    private:
        // Lexer
        const CharType *_start;
        const CharType *_end;
        const CharType *_current;
        TokenValue _value;
        bool IsSpace(CharType ch) { return ch <= 32; }
        bool IsNumber(CharType ch) { return ch >= '0' && ch <= '9'; }
        bool IsNonZeroNumber(CharType ch) { return ch >= '1' && ch <= '9'; }
        bool IsOperator(CharType ch);
        bool IsPrefix(CharType ch) { return ch == '+' || ch == '-' || ch == '!' || ch == '~'; }
        bool IsPostfix(CharType ch) { return ch == '\''; }
    };
    
    class Parser
    {
    public:
        typedef Lexer::CharType CharType;
        typedef Lexer::StringType StringType;
        typedef Lexer::TokenType TokenType;
    private:
        Lexer _lexer;
    public:
        Parser(const CharType *start, const CharType *end) : _lexer(start, end) {}
        void Parse();
    private:
         int CompareOperator(StringType op1, StringType op2);
   };

    int Main(const std::vector<std::wstring>& args);
}