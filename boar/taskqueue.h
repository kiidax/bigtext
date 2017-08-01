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
        typedef std::function<size_t(void*, size_t)> TaskFunctionType;
        typedef std::function<void(size_t count, void*, size_t)> ReduceFunctionType;

        static const size_t CHUNK_SIZE = 4 * 1024;

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
        static unsigned _GetDefaultNumThreads();
        static unsigned _GetDefaultMaxTasks();

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

        void AddTask(void* addr, size_t n);
        void Dispatch(const void* addr, size_t n);

    private:
        void _Run();
        TaskInfo* _Receive();
        TaskInfo* _SendOrReceive(TaskInfo job);
        int _NextTask();
        void _EndTask(int taskId);
    };
}
