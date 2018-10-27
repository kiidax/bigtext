/* Boar - Boar is a collection of tools to process large text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once
#include <exception>

namespace boar
{
    namespace fs = boost::filesystem;
    namespace ios = boost::iostreams;
    namespace rnd = boost::random;

    struct SampleOutputSpec
    {
    public:
        fs::path fileName;
        uintmax_t numberOfLines;
        double rate;

        SampleOutputSpec(const fs::path &fileName) : fileName(fileName), rate(1.0), numberOfLines(0) {}
        SampleOutputSpec(const fs::path &fileName, double rate) : fileName(fileName), rate(rate), numberOfLines(0) {}
        SampleOutputSpec(const fs::path &fileName, uintmax_t numberOfLines) : fileName(fileName), rate(0.0), numberOfLines(numberOfLines) {}
    };

    template <typename CharT>
    void FileLineSample(const std::vector<fs::path> &inputFileNameList, double rate, fs::path& outputFileName)
    {
        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::bernoulli_distribution<> dist(rate);

        fs::ofstream out;
        out.open(outputFileName, std::ios::out | std::ios::binary);
        if (!out.is_open())
        {
            std::wcerr << __wcserror(outputFileName.native().c_str());
            return;
        }

        for (auto& fileName : inputFileNameList)
        {
            FileLineSourceDefault<CharT>(fileName, [&dist, &gen, &out](const CharT *s, size_t len)
            {
                if (dist(gen))
                {
                    out.write(reinterpret_cast<const char *>(s), sizeof(CharT) * len);
                }
            });
        }
    }

    template <typename CharT>
    void FileLineSample(const std::vector<fs::path> &inputPathList, const std::vector<SampleOutputSpec> &outputSpecList)
    {
        struct OutputProgress
        {
            double randomThreshold;
            uintmax_t lineCount;
            fs::ofstream out;
        };

        size_t numOutputs;
        OutputProgress *outputProgressList;
        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::uniform_real_distribution<> dist(0, 1);

        numOutputs = outputSpecList.size();
        outputProgressList = new OutputProgress[numOutputs];

        for (size_t i = 0; i < numOutputs; i++)
        {
            auto& spec = outputSpecList[i];
            if (spec.numberOfLines != 0)
            {
                throw std::logic_error("Taget lines is not allowed.");
            }
            else if (spec.rate >= 0)
            {
                outputProgressList[i].randomThreshold = spec.rate;
            }
            else
            {
                throw std::logic_error("None of taget lines or rate is specified.");
            }
            auto &out = outputProgressList[i].out;
            out.open(spec.fileName);
            if (!out.is_open())
            {
                std::wcerr << __wcserror(spec.fileName.native().c_str());
                return;
            }
        }

        for (auto &fileName : inputPathList)
        {
            FileLineSourceDefault<CharT>(fileName, [&dist, &gen, outputProgressList, numOutputs](const CharT *s, size_t len)
            {
                double t = dist(gen);
                for (int i = 0; i < numOutputs; i++)
                {
                    auto &prog = outputProgressList[i];
                    if (t < prog.randomThreshold)
                    {
                        prog.out.write(reinterpret_cast<const char *>(s), sizeof(CharT) * len);
                        ++prog.lineCount;
                        break;
                    }
                    t -= prog.randomThreshold;
                }
            });
        }

        delete[] outputProgressList;
    }

    template<typename CharT>
    void FileShuffleLines(const std::vector<fs::path> &inputFileNameList, const std::vector<SampleOutputSpec> &outputSpecList)
    {
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
                std::wcerr << __wcserror(inputFileName.native().c_str());
                return;
            }

            const CharT *s = reinterpret_cast<const CharT *>(file.data());
            size_t len = file.size() / sizeof(CharT);
            std::wcout << inputFileName.native() << "\tCharCount\t" << len << std::endl;

            linePositionList.push_back(s);
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
        std::cout << "\tLineCount\t" << numLines << std::endl;
        if (lineIndex - inputFileNameList.size() != numLines)
        {
            std::wcerr << "something wrong" << std::endl;
            return;
        }

        rnd::mt19937_64 gen(std::time(nullptr));
        rnd::random_number_generator<rnd::mt19937_64, size_t> dist(gen);
        for (size_t i = 0; i < numLines - 1; i++)
        {
            size_t j = i + dist(numLines - i);
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
                std::wcerr << __wcserror(outputSpec.fileName.native().c_str());
                return;
            }

            for (uintmax_t i = 0; i < lineCount; i++)
            {
                if (curIndex >= numLines)
                {
                    break;
                }

                size_t n = lineIndexList[curIndex++];
                const CharT *first = linePositionList[n];
                const CharT *last = linePositionList[n + 1];
                out.write(first, last - first);
            }
        }
    }

    template<typename CharT>
    void FileShuffleLines(const std::vector<fs::path> &inputFileNameList, const std::vector<SampleOutputSpec> &outputSpecList, uintmax_t interleavingSize, size_t maxBufferSize)
    {
        std::wcout << "\tInterleavingSize\t" << interleavingSize << std::endl;
        std::vector<const CharT *> linePositionList;
        std::wcout << "\tMaxBufferSize\t" << maxBufferSize << std::endl;
        heap_vector<CharT> heap(maxBufferSize / sizeof(CharT));

        size_t lineIndex = 0;
        CharT *p = heap.ptr();
        CharT *last = heap.ptr() + maxBufferSize / sizeof(CharT);
        for (uintmax_t sliceStart = interleavingSize; sliceStart > 0; --sliceStart)
        {
            std::wcout << "\tCurrentSlice\t" << sliceStart << std::endl;
            uintmax_t currentSlice = sliceStart;
            uintmax_t lineCount = 0;

            for (auto &inputFileName : inputFileNameList)
            {
                linePositionList.push_back(p);
                std::wcout << inputFileName.native() << "\tReading" << std::endl;
                FileLineSourceDefault<CharT>(inputFileName, [&p, last, &currentSlice, &linePositionList, &lineCount, interleavingSize](const CharT *s, size_t len)
                {
                    if (len > 0)
                    {
                        if (--currentSlice == 0)
                        {
                            std::memcpy(p, s, len * sizeof(CharT));
                            p += len;
                            linePositionList.push_back(p);
                            currentSlice = interleavingSize;
                        }
                        lineCount++;
                    }
                });
            }

            // Shuffle lines

            std::vector<size_t> lineIndexList;
            uintmax_t numLines = linePositionList.size() - 1;
            std::wcout << "Shuffling " << numLines << " lines" << std::endl;
            std::wcout.flush();
            lineIndexList.resize(numLines);
            for (size_t i = 0; i != numLines; i++)
            {
                lineIndexList[i] = i;
            }

            rnd::mt19937_64 gen(std::time(nullptr));
            rnd::random_number_generator<rnd::mt19937_64, size_t> dist(gen);
            for (size_t i = 0; i < numLines - 1; i++)
            {
                size_t j = i + dist(numLines - i);
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

                std::wcerr << outputSpec.fileName.native() << "\tLineCount\t" << min(lineCount, numLines - curIndex) << std::endl;

                fs::basic_ofstream<CharT> out;
                int mode = std::ios::out | std::ios::binary;
                if (sliceStart != interleavingSize)
                {
                    mode |= std::ios::app;
                }
                out.open(outputSpec.fileName, mode);
                if (!out.is_open())
                {
                    std::wcerr << __wcserror(outputSpec.fileName.native().c_str());
                    return;
                }

                for (uintmax_t i = 0; i < lineCount; i++)
                {
                    if (curIndex >= numLines)
                    {
                        break;
                    }

                    size_t n = lineIndexList[curIndex++];
                    const CharT *first = linePositionList[n];
                    const CharT *last = linePositionList[n + 1];
                    out.write(first, last - first);
                }
            }

            linePositionList.clear();
        }
    }

    template <typename CharT>
    void FileQuickSampleFileLines(fs::path &inputFileName, const std::vector<SampleOutputSpec> &outputSpecList, bool shuffleOutput)
    {
        static_assert(sizeof(CharT) == sizeof(char), "Only char type is supported.");
        rnd::mt19937_64 gen(std::time(nullptr));
        uintmax_t fileSize = fs::file_size(inputFileName);
        rnd::uniform_int_distribution<std::streamoff> dist(0, fileSize);
        fs::ifstream fin(inputFileName);
        if (!fin.is_open())
        {
            return;
        }
        for (auto &spec : outputSpecList)
        {
            std::vector<std::basic_string<CharT>> lineList;
            for (int i = 0; i < spec.numberOfLines; i++)
            {
                uintmax_t pos = dist(gen);
                fin.seekg(pos);
                std::basic_string<CharT> line;
                std::getline(fin, line);
                if (fin.eof())
                {
                    fin.clear();
                    fin.seekg(0);
                }
                std::getline(fin, line);
                if (fin.eof())
                {
                    fin.clear();
                }
                lineList.push_back(std::move(line));
            }
            fs::ofstream fout(spec.fileName, std::ios::out | std::ios::binary);
            for (auto &line : lineList)
            {
                fout << line << '\n';
            }
        }
    }
}