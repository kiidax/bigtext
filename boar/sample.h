/* Boar - Boar is a collection of tools to process text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

namespace boar
{
    namespace fs = boost::filesystem;
    namespace ios = boost::iostreams;
    namespace rnd = boost::random;

    struct OutputSpec
    {
        uintmax_t numberOfLines;
        double rate;
        fs::path fileName;

    public:
        OutputSpec(const fs::path &fileName, double rate) : fileName(fileName), rate(rate), numberOfLines(0) {}
        OutputSpec(const fs::path &fileName, uintmax_t numberOfLines) : fileName(fileName), rate(0.0), numberOfLines(numberOfLines) {}
    };

    template <typename CharT>
    void FileLineSample(const std::vector<fs::path> &inputFileNameList, double rate, fs::path& outputFileName)
    {
        fs::ofstream _out;
        rnd::mt19937_64 _gen;
        rnd::bernoulli_distribution<> _dist;

        _dist = rnd::bernoulli_distribution<>(rate);
        _out.open(outputFileName, std::ios::out | std::ios::binary);
        if (!_out.is_open())
        {
            std::wcerr << "cannot open" << std::endl;
            return;
        }
        for (auto& fileName : inputFileNameList)
        {
            FileLineSourceDefault<CharT>(fileName, [&_dist, &_gen, &_out](const CharT *s, size_t len)
            {
                if (_dist(_gen))
                {
                    _out.write(reinterpret_cast<const char *>(s), sizeof(CharT) * len);
                }
            });
        }
    }

    template <typename CharT>
    class LineSampleProcessor2
    {
    public:
        typedef CharT CharType;

    protected:
        struct OutputProgress
        {
            double randomThreshold;
            uintmax_t lineCount;
            fs::ofstream out;
        };

    public:
        LineSampleProcessor2() : _gen(std::time(nullptr)), _dist(0.0, 1.0)
        {
        }

    protected:
        size_t _numOutputs;
        OutputProgress *_outputProgressList;
        rnd::mt19937_64 _gen;
        rnd::uniform_real_distribution<> _dist;

    public:
        void Run(const std::vector<fs::path>& inputPathList, const std::vector<OutputSpec>& outputSpecList)
        {
            _numOutputs = outputSpecList.size();
            _outputProgressList = new OutputProgress[_numOutputs];

            for (size_t i = 0; i < _numOutputs; i++)
            {
                auto& spec = outputSpecList[i];
                if (spec.numberOfLines > 0)
                {
                    // TODO: overflow
                    _outputProgressList[i].randomThreshold = 0.0;
                }
                else if (spec.rate >= 0)
                {
                    _outputProgressList[i].randomThreshold = spec.rate;
                }
                else
                {
                    return;
                }
                auto &out = _outputProgressList[i].out;
                out.open(spec.fileName);
                if (!out.is_open())
                {
                    std::wcerr << "cannot open" << std::endl;
                    return;
                }
            }

            for (auto &fileName : inputPathList)
            {
                boar::FileLineSourceDefault<LineSampleProcessor2<CharT>, CharT>(fileName, *this);
            }
        }

        void ProcessLine(const CharT *s, size_t len)
        {
            double t = _dist(_gen);
            for (int i = 0; i < _numOutputs; i++)
            {
                auto &prog = _outputProgressList[i];
                if (t < prog.randomThreshold)
                {
                    prog.out.write(reinterpret_cast<const char *>(s), sizeof (CharT) * len);
                    ++prog.lineCount;
                }
                t -= prog.randomThreshold;
            }
        }
    };

    template<typename CharT>
    void FileShuffleLines(const std::vector<fs::path> &inputFileNameList, const std::vector<OutputSpec> &outputSpecList)
    {
        rnd::mt19937_64 gen(std::time(nullptr));

        std::vector<size_t> lineIndexList;
        std::vector<const CharT *> linePositionList;

        std::vector<ios::mapped_file_source> fileList;
        size_t lineIndex = 0;
        for (auto &inputFileName : inputFileNameList)
        {
            size_t prevLineIndex = lineIndex;
            fileList.emplace_back();
            auto &file = fileList.back();
            file.open(inputFileName);
            if (!file.is_open())
            {
                std::wcerr << "error" << std::endl;
                return;
            }

            const CharT *s = reinterpret_cast<const CharT *>(file.data());
            size_t len = file.size() / sizeof(CharT);

            linePositionList.push_back(s);
            std::wcout << inputFileName.native() << "\tCharCount\t" << len << std::endl;
            for (size_t i = 0; i < len; i++)
            {
                if (s[i] == '\n')
                {
                    lineIndexList.push_back(lineIndex++);
                    linePositionList.push_back(&s[i + 1]);
                }
            }
            if (s[len - 1] != '\n')
            {
                lineIndexList.push_back(lineIndex++);
                linePositionList.push_back(&s[len]);
            }

            std::wcout << inputFileName.native() << "\tLineCount\t" << (lineIndex - prevLineIndex) << std::endl;
            lineIndex++;
        }

        // Shuffle lines

        size_t numLines = lineIndexList.size();
        if (lineIndex - inputFileNameList.size() != numLines)
        {
            std::wcerr << "something wrong" << std::endl;
        }
        std::cout << "\tLineCount\t" << numLines << std::endl;
        rnd::random_number_generator<rnd::mt19937_64, size_t> dist(gen);
        for (size_t i = 0; i < numLines - 1; i++)
        {
            size_t j = dist(numLines - i);
            std::swap(lineIndexList[i], lineIndexList[j]);
        }

        // Write lines

        size_t curIndex = 0;
        for (auto &outputSpec : outputSpecList)
        {
            uintmax_t lineCount;
            if (outputSpec.numberOfLines > 0)
            {
                lineCount = outputSpec.numberOfLines;
            }
            else if (outputSpec.rate >= 1.0)
            {
                lineCount = numLines;
            }
            else
            {
                lineCount = static_cast<uintmax_t>(numLines * outputSpec.rate + 0.5);
            }

            std::wcerr << outputSpec.fileName << "\tLineCount\t" << min(lineCount, numLines - curIndex) << std::endl;

            fs::basic_ofstream<CharT> out;
            out.open(outputSpec.fileName, std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                std::wcerr << "write error" << std::endl;
                return;
            }

            for (uintmax_t i = 0; i < lineCount; i++)
            {
                if (curIndex >= numLines)
                {
                    break;
                }

                size_t n = lineIndexList[curIndex++];
                const CharT *first = linePositionList[curIndex];
                const CharT *last = linePositionList[n + 1];
                out.write(first, last - first);
            }
        }
    }
}