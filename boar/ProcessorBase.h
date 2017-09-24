/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    class ProcessorBase
    {
    protected:
        std::wstring _currentFilePath;

    public:
        ProcessorBase();
        virtual ~ProcessorBase();

        void ProcessFileList(const std::vector<std::wstring>& filePathList);
        void ProcessFile(const std::wstring& filePath);
        virtual void BeginContent();
        virtual void ProcessBuffer(const void* first, const void* last) = 0;
        virtual void EndContent();
    };
}
