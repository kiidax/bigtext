/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "stdafx.h"
#include "LineProcessor.h"
#include <random>

namespace boar
{
    namespace fs = boost::filesystem;

    template <typename charT>
    class LineSampleProcessor : public LineProcessor<charT>
    {
    private:
        int _threshold;

    public:
        LineSampleProcessor(double rate)
        {
            _threshold = static_cast<int>(rate * RAND_MAX + 0.5);
        }

        virtual bool ProcessLine(const charT* first, const charT* last)
        {
            if (std::rand() < _threshold)
            {
                OutputText(first, last);
            }
            return true;
        }
    };

    template <typename charT>
    class LineSampleProcessor2
    {
    public:
        struct SampleSpec
        {
            boost::uintmax_t numberOfLines;
            double rate;
            fs::path path;
        };

    protected:
        struct SampleProgress
        {
            double rateOrNumberOfLines;
            boost::uintmax_t lineCount;
            LineFileWriter<charT> writer;
        };

    protected:
        size_t _numProgress;
        SampleProgress *_progressList;

    public:
        void Run(const std::vector<fs::path>& inputPathList, const std::vector<SampleSpec>& sampleSpecList, bool overwrite)
        {
            if (CheckInputFile(inputPathList))
            {
                std::cerr << "can't read." << std::endl;
            }

            if (!overwrite && CheckOutputFile(sampleSpecList))
            {
                std::cerr << "can't write." << std::endl;
            }

            _numProgress = sampleSpecList.size();
            _progressList = new SampleProgress[_numProgress];

            for (size_t i = 0; i < _numProgress; i++)
            {
                auto& spec = sampleSpecList[i];
                if (spec.numberOfLines > 0)
                {
                    // TODO: overflow
                    _progressList[i].rateOrNumberOfLines = -static_cast<double>(spec.numberOfLines);
                }
                else if (spec.rate >= 0)
                {
                    _progressList[i].rateOrNumberOfLines = spec.rate;
                }
                else
                {
                    return;
                }
                _progressList[i].writer.Open(spec.path);
            }

            std::for_each(inputPathList.cbegin(), inputPathList.cend(), [](auto &path)
            {
                FileSourceWithOverlapRead(path, [](const void *first, const void *last)
                {

                });
            });
        }

    private:
        bool CheckInputFile(const std::vector<fs::path>& inputPathList) const
        {
            return std::all_of(inputPathList.cbegin(), inputPathList.cend(), [](auto &path)
            {
                return fs::is_regular_file(path);
            });
        }

        bool CheckOutputFile(const std::vector<SampleSpec>& sampleSpecList) const
        {
            return std::all_of(sampleSpecList.cbegin(), sampleSpecList.cend(), [](auto &spec)
            {
                return !fs::exists(spec.path);
            });
        }
    };
}