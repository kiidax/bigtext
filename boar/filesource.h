/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include <boost/filesystem.hpp>

namespace boar
{
    typedef std::function<void(const void*, size_t)> DataSourceCallbackType;

    void FileSourceWithMemoryMapping(const boost::filesystem::path& fileName, DataSourceCallbackType callback);
    void FileSourceWithFileRead(const boost::filesystem::path& fileName, DataSourceCallbackType callback);
    void FileSourceWithOverlapRead(const boost::filesystem::path& fileName, DataSourceCallbackType callback);
}
