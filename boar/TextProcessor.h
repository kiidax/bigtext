#pragma once

namespace boar
{
    class TextProcessor
    {
    protected:
        std::wstring _currentFilePath;

    public:
        TextProcessor();
        virtual ~TextProcessor();

        void ProcessFileList(const std::vector<std::wstring>& filePathList);
        void ProcessFile(const std::wstring& filePath);
        virtual void BeginContent(const std::wstring& filePath);
        virtual void ProcessBuffer(const void *data, size_t n) = 0;
        virtual void EndContent();
    };
}
