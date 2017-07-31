/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#pragma once

#include "base.h"

namespace boar
{
    struct TaskInfo
    {
        void* addr;
        size_t n;
        size_t lineCount;
        bool done;
    };

    class TaskQueue
    {
    public:
        typedef std::function<void(void*, size_t, size_t&)> TaskFunctionType;
        typedef std::function<void(size_t count, void*, size_t)> ReduceFunctionType;

    private:
        static const int _INVALID_TASKID = -1;

        std::mutex _mutex;
        std::condition_variable _condEmpty;
        std::condition_variable _condFull;

        // Circular buffer
        TaskInfo* _tasks;
        int _maxTasks;
        int _taskOffset;
        int _numTasks;
        int _numWaitingTasks;

        boost::thread_group _threadGroup;
        unsigned _numThreads;
        bool _done;

        TaskFunctionType _taskFunc;
        ReduceFunctionType _reduceFunc;

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
        TaskQueue(TaskFunctionType taskFunc, ReduceFunctionType reduceFunc)
            : TaskQueue(taskFunc, reduceFunc, _GetDefaultNumThreads(), _GetDefaultMaxTasks()) {}
        TaskQueue(TaskFunctionType taskFunc, ReduceFunctionType reduceFunc, unsigned numThreads, unsigned maxTasks)
            : _taskFunc(taskFunc), _reduceFunc(reduceFunc), _numThreads(numThreads), _done(false)
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
                _threadGroup.create_thread(std::bind(&TaskQueue::_Run, this));
            }
        }
        void Stop()
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _done = true;
                _condEmpty.notify_all();
            }
            _threadGroup.join_all();
        }
        void Synchronize()
        {
            TaskInfo* task;
            while ((task = _Receive()) != nullptr)
            {
                // do something on task.
                _reduceFunc(task->lineCount, task->addr, task->n);
            }
        }
        void AddTask(void* addr, size_t n)
        {
            TaskInfo* task;
            TaskInfo newTask;
            newTask.addr = addr;
            newTask.n = n;
            newTask.done = false;
            while ((task = _SendOrReceive(newTask)) != nullptr)
            {
                // do something on task.
                _reduceFunc(task->lineCount, task->addr, task->n);
            }
        }
    private:
        void _Run()
        {
            while (true)
            {
                int taskId = _NextTask();
                if (taskId == _INVALID_TASKID) break;
                TaskInfo& task = _tasks[taskId];
                _taskFunc(task.addr, task.n, task.lineCount);
                _EndTask(taskId);
            }
        }
        TaskInfo* _Receive()
        {
            std::unique_lock<std::mutex> lock(_mutex);
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
                if (_numTasks > 0)
                {
                    _condFull.wait(lock);
                }
                else
                {
                    return nullptr;
                }
            }
        }
        TaskInfo* _SendOrReceive(TaskInfo job)
        {
            std::unique_lock<std::mutex> lock(_mutex);
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
            std::unique_lock<std::mutex> lock(_mutex);
            while (true)
            {
                if (_numWaitingTasks == 0)
                {
                    if (_done) break;
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
        void _EndTask(int taskId)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks[taskId].done = true;
            if (taskId == _taskOffset)
            {
                _condFull.notify_one();
            }
        }
    };
}
