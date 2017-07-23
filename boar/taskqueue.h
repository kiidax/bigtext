/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"

namespace boar
{
    class TaskQueue
    {
    private:
        typedef boost::function<void(void)> TaskFunction;
        boost::mutex _mutex;
        boost::condition_variable _condEmpty;
        boost::condition_variable _condFull;
        boost::circular_buffer<TaskFunction> _buffer;
        boost::thread_group _threadGroup;
        unsigned _numThreads;
        bool _done;

    private:
        static unsigned _GetDefaultNumThreads()
        {
            unsigned numHardwareConcurrency = boost::thread::hardware_concurrency();
            if (numHardwareConcurrency == 0) return 4;
            return (numHardwareConcurrency + 1) * 6 / 5;
        }
        static unsigned _GetDefaultMaxTasks()
        {
            return _GetDefaultNumThreads() * 5;
        }

    public:
        TaskQueue() : TaskQueue(_GetDefaultNumThreads(), _GetDefaultMaxTasks()) {}
        TaskQueue(unsigned numThreads, unsigned maxJobs) : _buffer(maxJobs), _numThreads(numThreads), _done(false)
        {
        }
        ~TaskQueue()
        {
            {
                boost::mutex::scoped_lock lock(_mutex);
                _done = true;
                _condEmpty.notify_all();
            }
            _threadGroup.join_all();
        }
        void Start()
        {
            for (unsigned i = 0; i < _numThreads; i++)
            {
                _threadGroup.create_thread([this]() {
                    while (true)
                    {
                        TaskFunction job;
                        if (!_PopTask(job)) break;
                        job();
                    }
                });
            }
        }
        void PushTask(TaskFunction job)
        {
            boost::mutex::scoped_lock lock(_mutex);
            if (_buffer.full())
            {
                _condFull.wait(lock);
            }
            bool doNotify = _buffer.empty();
            _buffer.push_back(job);
            if (doNotify) _condEmpty.notify_one();
        }
    private:
        bool _PopTask(TaskFunction& job)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (!_done && _buffer.empty())
                {
                    _condEmpty.wait(lock);
                }
                else
                {
                    if (_done) return false;
                    job = _buffer[0];
                    bool doNotify = _buffer.full();
                    _buffer.pop_front();
                    if (doNotify) _condFull.notify_one();
                    return true;
                }
            }
        }
    };
}
