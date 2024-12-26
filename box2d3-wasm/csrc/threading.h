// TaskSystem.h
#ifndef TASK_SYSTEM_H
#define TASK_SYSTEM_H

#include "TaskScheduler.h"
#include <box2d/box2d.h>
#include <cstdint>
#include <cassert>

class PhysicsTask : public enki::ITaskSet
{
public:
    PhysicsTask() = default;

    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadIndex) override
    {
        if (taskCallback)
        {
            taskCallback(range.start, range.end, threadIndex, taskData);
        }
    }

    b2TaskCallback* taskCallback = nullptr;
    void* taskData = nullptr;
};

struct TaskSystem
{
    static constexpr int MAX_TASKS = 128;

    enki::TaskScheduler* taskScheduler = nullptr;
    PhysicsTask* tasks = nullptr;
    int activeTaskCount = 0;
    int totalThreadCount = 0;

    explicit TaskSystem(int workerCount)
    {
        taskScheduler = new enki::TaskScheduler;
        taskScheduler->Initialize(workerCount);

        tasks = new PhysicsTask[MAX_TASKS];
        activeTaskCount = 0;

        totalThreadCount = 1 + workerCount;
    }

    ~TaskSystem()
    {
        if (taskScheduler)
        {
            taskScheduler->WaitforAll();
            delete taskScheduler;
        }
        delete[] tasks;
    }

    void clearTasks()
    {
        activeTaskCount = 0;
    }

    int getActiveTaskCount() const
    {
        return activeTaskCount;
    }

    int getTotalThreadCount() const
    {
        return totalThreadCount;
    }
};

#endif // TASK_SYSTEM_H
