/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include <string>

namespace boar {
    class Parser
    {
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
    private:
        const CharType *_start;
        const CharType *_end;
        const CharType *_current;
        TokenValue _value;
        bool IsSpace(CharType ch) { return ch <= 32; }
        bool IsNumber(CharType ch) { return ch >= '0' && ch <= '9'; }
        bool IsNonZeroNumber(CharType ch) { return ch >= '1' && ch <= '9'; }
        TokenType Lex();
    public:
        Parser(const CharType *start, const CharType *end);
        void Parse();
    };

    int Main(const std::vector<std::wstring>& args);
}