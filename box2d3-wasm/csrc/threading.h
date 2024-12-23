#ifndef SAMPLE_H
#define SAMPLE_H

#include "TaskScheduler.h"
#include <box2d/box2d.h>
#include <cstdint>
#include <cassert>

class SampleTask : public enki::ITaskSet
{
public:
    SampleTask() = default;

    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadIndex) override
    {
        if (m_task)
        {
            m_task(range.start, range.end, threadIndex, m_taskContext);
        }
    }

    b2TaskCallback* m_task = nullptr;
    void* m_taskContext = nullptr;
};

struct Sample
{
    static constexpr int m_maxTasks = 128;

    enki::TaskScheduler* m_scheduler = nullptr;
    SampleTask* m_tasks = nullptr;
    int m_taskCount = 0;
    int m_threadCount = 0;

    explicit Sample(int workerCount)
    {
        m_scheduler = new enki::TaskScheduler;
        m_scheduler->Initialize(workerCount);

        m_tasks = new SampleTask[m_maxTasks];
        m_taskCount = 0;

        m_threadCount = 1 + workerCount;
    }

    ~Sample()
    {
        if (m_scheduler)
        {
            m_scheduler->WaitforAll();
            delete m_scheduler;
        }
        delete[] m_tasks;
    }

    void resetTaskCount()
    {
        m_taskCount = 0;
    }

    int getTaskCount() const
    {
        return m_taskCount;
    }

    int getThreadCount() const
    {
        return m_threadCount;
    }
};

#endif // SAMPLE_H
