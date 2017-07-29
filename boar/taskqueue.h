/* Boar - Boar is a toolkit to modify text files.
* Copyright (C) 2017 Katsuya Iida. All rights reserved.
*/

#pragma once

#include "base.h"

namespace boar
{
    typedef boost::function<void(void)> TaskFunction;

    struct TaskInfo
    {
        TaskFunction func;
        bool done;
    };

    class TaskQueue
    {
    private:
        boost::mutex _mutex;
        boost::condition_variable _condEmpty;
        boost::condition_variable _condFull;
        boost::circular_buffer<TaskInfo> _buffer;
        int _aaa;
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
        TaskQueue(unsigned numThreads, unsigned maxJobs) : _buffer(maxJobs), _aaa(0), _numThreads(numThreads), _done(false)
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
                        boost::circular_buffer<TaskInfo>::iterator it;
                        if (!_NextTask(it)) break;
                        it->func();
                        {
                            boost::mutex::scoped_lock lock(_mutex);
                            it->done = true;
                            _condFull.notify_one();
                        }
                    }
                });
            }
        }
        void PushTask(TaskInfo job)
        {
            TaskInfo task;
            while (!_SendOrReceive(job, task))
            {
            }
        }
    private:
        bool _SendOrReceive(TaskInfo job, TaskInfo& task)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (!_buffer.empty() && _buffer[0].done)
                {
                    task = _buffer[0];
                    _buffer.pop_front();
                    _aaa--;
                    return false;
                }
                if (_buffer.full())
                {
                    _condFull.wait(lock);
                }
                else
                {
                    bool doNotify = _aaa == _buffer.size();
                    _buffer.push_back(job);
                    if (doNotify) _condEmpty.notify_one();
                    return true;
                }
            }
        }
        bool _NextTask(boost::circular_buffer<TaskInfo>::iterator& it)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (_done) break;
                if (_aaa == _buffer.size())
                {
                    _condEmpty.wait(lock);
                }
                else
                {
                    it = _buffer.begin() + _aaa;
                    ++_aaa;
                    return true;
                }
            }
            return false;
        }
    };
}
