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
        static const int _INVALID_TASKID = -1;

        boost::mutex _mutex;
        boost::condition_variable _condEmpty;
        boost::condition_variable _condFull;

        // Circular buffer
        TaskInfo* _tasks;
        int _maxTasks;
        int _taskOffset;
        int _numTasks;
        int _numWaitingTasks;

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
            return _GetDefaultNumThreads() * 3;
        }

    public:
        TaskQueue() : TaskQueue(_GetDefaultNumThreads(), _GetDefaultMaxTasks()) {}
        TaskQueue(unsigned numThreads, unsigned maxTasks) : _numThreads(numThreads), _done(false)
        {
            _tasks = new TaskInfo[maxTasks];
            _maxTasks = maxTasks;
            _taskOffset = 0;
            _numTasks = 0;
            _numWaitingTasks = 0;
        }
        ~TaskQueue()
        {
            assert(_done);
            delete[] _tasks;
        }
        void Start()
        {
            for (unsigned i = 0; i < _numThreads; i++)
            {
                _threadGroup.create_thread(std::bind(&TaskQueue::Run, this));
            }
        }
        void Stop()
        {
            {
                boost::mutex::scoped_lock lock(_mutex);
                _done = true;
                _condEmpty.notify_all();
            }
            _threadGroup.join_all();
        }
        void PushTask(TaskInfo job)
        {
            TaskInfo* task;
            while ((task = _SendOrReceive(job)) != nullptr)
            {
                // do something on task.
            }
        }
    private:
        void Run()
        {
            while (true)
            {
                int taskId = _NextTask();
                if (taskId == _INVALID_TASKID) break;
                _tasks[taskId].func();
                {
                    boost::mutex::scoped_lock lock(_mutex);
                    _tasks[taskId].done = true;
                    if (taskId == _taskOffset)
                    {
                        _condFull.notify_one();
                    }
                }
            }
        }
        TaskInfo* _SendOrReceive(TaskInfo job)
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (_numWaitingTasks < _numTasks)
                {
                    int taskId = _taskOffset;
                    if (_tasks[taskId].done)
                    {
                        // This pointer is only valid until we call _SendOrRecive() again.
                        TaskInfo* doneTask = &_tasks[taskId];
                        ++_taskOffset;
                        if (_taskOffset >= _maxTasks) _taskOffset -= _maxTasks;
                        --_numTasks;
                        return doneTask;
                    }
                }
                if (_numTasks >= _maxTasks)
                {
                    _condFull.wait(lock);
                }
                else
                {
                    bool doNotify = _numWaitingTasks == 0;
                    int taskId = _taskOffset + _numTasks;
                    if (taskId >= _maxTasks) taskId -= _maxTasks;
                    ++_numTasks;
                    ++_numWaitingTasks;
                    _tasks[taskId] = job;
                    if (doNotify) _condEmpty.notify_one();
                    return nullptr;
                }
            }
        }
        int _NextTask()
        {
            boost::mutex::scoped_lock lock(_mutex);
            while (true)
            {
                if (_done) break;
                if (_numWaitingTasks == 0)
                {
                    _condEmpty.wait(lock);
                }
                else
                {
                    int taskId = _taskOffset + _numTasks - _numWaitingTasks;
                    if (taskId >= _maxTasks) taskId -= _maxTasks;
                    --_numWaitingTasks;
                    return taskId;
                }
            }
            return _INVALID_TASKID;
        }
    };
}
