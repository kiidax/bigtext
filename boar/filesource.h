/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"
#include <boost/filesystem.hpp>

namespace boar
{
    typedef std::function<void(const void*, size_t)> DataSourceCallbackType;

    void FileSource(const boost::filesystem::path& filename, DataSourceCallbackType func);
    void FileSource2(const boost::filesystem::path& filename, DataSourceCallbackType func);
    void FileSource3(const boost::filesystem::path& filename, DataSourceCallbackType func);
}
