// TaskSystem.cpp
#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include "threading.h"
#include "TaskScheduler.h"

using namespace emscripten;
using namespace enki;
using namespace b2;

static void* SchedulePhysicsTask(b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* userContext)
{
    TaskSystem* system = static_cast<TaskSystem*>(userContext);

    if (system->activeTaskCount < TaskSystem::MAX_TASKS)
    {
        PhysicsTask& physicsTask = system->tasks[system->activeTaskCount];
        physicsTask.m_SetSize = itemCount;
        physicsTask.m_MinRange = minRange;
        physicsTask.taskCallback = task;
        physicsTask.taskData = taskContext;

        system->taskScheduler->AddTaskSetToPipe(&physicsTask);
        ++system->activeTaskCount;
        return &physicsTask;
    }
    else
    {
        task(0, itemCount, 0, taskContext);
        return nullptr;
    }
}

static void WaitForPhysicsTask(void* taskPtr, void* userContext)
{
    if (taskPtr != nullptr)
    {
        PhysicsTask* physicsTask = static_cast<PhysicsTask*>(taskPtr);
        TaskSystem* system = static_cast<TaskSystem*>(userContext);
        system->taskScheduler->WaitforTask(physicsTask);
    }
}

EMSCRIPTEN_BINDINGS(threading) {
    class_<TaskSystem>("TaskSystem")
        .constructor<int>()
        .function("getActiveTaskCount", &TaskSystem::getActiveTaskCount)
        .function("getTotalThreadCount", &TaskSystem::getTotalThreadCount)
        .function("clearTasks", &TaskSystem::clearTasks)
        ;

    function("b2CreateThreadedWorld", optional_override([](
        b2WorldDef& originalDef,
        TaskSystem& system
    ) -> b2WorldId
    {
        b2WorldDef def = originalDef;
        def.enqueueTask = &SchedulePhysicsTask;
        def.finishTask = &WaitForPhysicsTask;
        def.userTaskContext = &system;
        def.workerCount = system.getTotalThreadCount() - 1;

        return b2CreateWorld(&def);
    }));
}
