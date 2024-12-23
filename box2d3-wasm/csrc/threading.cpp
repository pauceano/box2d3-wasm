#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include "threading.h"
#include "TaskScheduler.h"


using namespace emscripten;
using namespace enki;
using namespace b2;

static void* EnqueueTask(b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* userContext)
{
    Sample* sample = static_cast<Sample*>(userContext);
    
    if (sample->m_taskCount < Sample::m_maxTasks)
    {
        SampleTask& sampleTask = sample->m_tasks[sample->m_taskCount];
        sampleTask.m_SetSize = itemCount;
        sampleTask.m_MinRange = minRange;
        sampleTask.m_task = task;
        sampleTask.m_taskContext = taskContext;

        sample->m_scheduler->AddTaskSetToPipe(&sampleTask);
        ++sample->m_taskCount;
        return &sampleTask;
    }
    else
    {
        task(0, itemCount, 0, taskContext);
        return nullptr;
    }
}

static void FinishTask(void* taskPtr, void* userContext)
{
    if (taskPtr != nullptr)
    {
        SampleTask* sampleTask = static_cast<SampleTask*>(taskPtr);
        Sample* sample = static_cast<Sample*>(userContext);
        sample->m_scheduler->WaitforTask(sampleTask);
    }
}

EMSCRIPTEN_BINDINGS(box2d_setup) {
	class_<Sample>("Sample")
    	.constructor<int>()
        .function("getTaskCount", &Sample::getTaskCount)
        .function("getThreadCount", &Sample::getThreadCount)
        .function("resetTaskCount", &Sample::resetTaskCount)
		;


	function("createThreadedSampleWorld", optional_override([](
		b2WorldDef& originalDef,
		Sample& sample
	) -> b2WorldId
	{
        b2WorldDef def = originalDef;
        def.enqueueTask = &EnqueueTask;
        def.finishTask = &FinishTask;
        def.userTaskContext = &sample;
        def.workerCount = sample.getThreadCount() - 1;

        printf("Creating threaded world with %d workers\n", def.workerCount);

        return b2CreateWorld(&def);
	}));
}
