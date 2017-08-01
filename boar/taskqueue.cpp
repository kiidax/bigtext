/* Boar - Boar is a toolkit to modify text files.
 * Copyright (C) 2017 Katsuya Iida. All rights reserved.
 */

#include "stdafx.h"

#include "boar.h"
#include "taskqueue.h"

namespace boar
{
    unsigned TaskQueue::_GetDefaultNumThreads()
    {
        unsigned numHardwareConcurrency = boost::thread::hardware_concurrency();
        if (numHardwareConcurrency == 0) return 4;
        return (numHardwareConcurrency + 1) * 6 / 5;
    }

    unsigned TaskQueue::_GetDefaultMaxTasks()
    {
        return _GetDefaultNumThreads() * 30;
    }

    void TaskQueue::AddTask(void* addr, size_t n)
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

    void TaskQueue::Dispatch(const void *addr, size_t n)
    {
        for (size_t i = 0; i < n; i += CHUNK_SIZE)
        {
            intptr_t chunkAddr = reinterpret_cast<intptr_t>(addr) + i;
            size_t chunkSize = n - i < CHUNK_SIZE ? n - i : CHUNK_SIZE;
            AddTask(reinterpret_cast<void*>(chunkAddr), chunkSize);
        }
        Synchronize();
    }

    void TaskQueue::_Run()
    {
        while (true)
        {
            int taskId = _NextTask();
            if (taskId == _INVALID_TASKID) break;
            TaskInfo& task = _tasks[taskId];
            task.lineCount = _taskFunc(task.addr, task.n);
            _EndTask(taskId);
        }
    }
    TaskInfo* TaskQueue::_Receive()
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
    TaskInfo* TaskQueue::_SendOrReceive(TaskInfo job)
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
    int TaskQueue::_NextTask()
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
    void TaskQueue::_EndTask(int taskId)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _tasks[taskId].done = true;
        if (taskId == _taskOffset)
        {
            _condFull.notify_one();
        }
    }
}