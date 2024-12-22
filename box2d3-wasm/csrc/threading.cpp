#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include "TaskScheduler.h"

using namespace emscripten;
using namespace enki;
using namespace b2;

// define a task set, can ignore range if we only do one thing
struct B2DTaskSet : ITaskSet {
    using ITaskSet::ITaskSet;

    struct TaskData {
        int32_t start;
        int32_t end;
        void* context;
    };

private:
    std::vector<TaskData> data;
    b2TaskCallback* currentTask = nullptr;

public:
    void Init(uint32_t num_threads) {
        printf("B2DTaskSet::Init with %u threads\n", num_threads);
        EnsureSize(num_threads);
    }

    void EnsureSize(uint32_t num_threads) {
        data.resize(num_threads);
    }

    void SetTask(b2TaskCallback* task) {
        currentTask = task;
    }

    void Assign(uint32_t index, int32_t start, int32_t end, void* context) {
        data[index] = TaskData{
            .start = start,
            .end = end,
            .context = context
        };
    }

    void ExecuteRange(TaskSetPartition range_, uint32_t threadnum_) override {
        if (currentTask && threadnum_ < data.size()) {
            // Use modulo to ensure thread index stays within bounds
            uint32_t safeThreadIdx = threadnum_ % data.size();
            const TaskData& taskData = data[safeThreadIdx];
            currentTask(taskData.start, taskData.end, safeThreadIdx, taskData.context);
            printf("Executed task on thread %u (safe index: %u)\n", threadnum_, safeThreadIdx);
        }
    }
};

struct TaskManager {
    TaskScheduler& sched;
    B2DTaskSet& taskSet;
    uint32_t maxTasks;
    uint32_t taskCount = 0;

    TaskManager(TaskScheduler& sched, B2DTaskSet& taskSet, uint32_t maxTasks = 128)
        : sched(sched)
        , taskSet(taskSet)
        , maxTasks(maxTasks)
    {
        printf("TaskManager constructed with maxTasks: %d\n", maxTasks);
    }

  void* EnqueueTask(b2TaskCallback* box2dTask, int itemCount, int minRange, void* box2dContext, [[maybe_unused]] void* userContext)
    {
        printf("EnqueueTask - itemCount: %d, minRange: %d, taskCount: %d\n", 
            itemCount, minRange, taskCount);
        
        if (taskCount >= maxTasks) {
            printf("Maximum tasks reached (%d), executing synchronously\n", maxTasks);
            box2dTask(0, itemCount, 0, box2dContext);
            return NULL;
        }

        // Wait for any previous task to complete
        sched.WaitforTask(&taskSet);

        taskSet.SetTask(box2dTask);
        taskSet.Assign(0, 0, itemCount, box2dContext);  // Simplified for now

        taskSet.m_MinRange = minRange;
        taskSet.m_SetSize = itemCount;

        sched.AddTaskSetToPipe(&taskSet);
        
        ++taskCount;
        printf("Task enqueued successfully, new taskCount: %d\n", taskCount);
        return &taskSet;
    }

    void FinishTask(void* userTask, [[maybe_unused]] void* userContext) {
        printf("FinishTask called\n");
        if (userTask) {
            sched.WaitforTask(reinterpret_cast<B2DTaskSet*>(userTask));
            --taskCount;
        }
        printf("Task completed, remaining tasks: %d\n", taskCount);
    }

    static void* StaticEnqueueTask(b2TaskCallback* box2dTask, int itemCount, int minRange, void* box2dContext, void* userContext) {
        printf("StaticEnqueueTask - itemCount: %d, minRange: %d\n", itemCount, minRange);
        TaskManager* tm{reinterpret_cast<TaskManager*>(userContext)};
        void* result = tm->EnqueueTask(box2dTask, itemCount, minRange, box2dContext, userContext);
        printf("StaticEnqueueTask completed, result: %p\n", result);
        return result;
    }

    static void StaticFinishTask(void* userTask, void* userContext) {
        printf("StaticFinishTask called with userTask: %p\n", userTask);
        TaskManager* tm{reinterpret_cast<TaskManager*>(userContext)};
        tm->FinishTask(userTask, userContext);
        printf("StaticFinishTask completed\n");
    }
};

// your JS demo should try to do steps like these:
static void demo( int workerCount )
{
	TaskScheduler sched{};
	TaskSchedulerConfig config{sched.GetConfig()};
	config.numTaskThreadsToCreate = workerCount - 1;
	sched.Initialize(config);
	B2DTaskSet taskSet{};
	taskSet.Init(sched.GetNumTaskThreads());

	uint32_t maxTasks = 128;
	TaskManager tm{sched, taskSet, maxTasks};

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.enqueueTask = &TaskManager::StaticEnqueueTask;
	worldDef.finishTask = &TaskManager::StaticFinishTask;
	worldDef.userTaskContext = &tm;
	worldDef.workerCount = workerCount;
	worldDef.enableSleep = false;

	b2WorldId worldId = b2CreateWorld( &worldDef );
}

struct Box2DThreading {
    TaskScheduler sched;
    B2DTaskSet taskSet;
    TaskManager tm;
    uint32_t numThreads;

    Box2DThreading(uint32_t maxTasks, uint32_t numThreads_)
        : sched{}
        , taskSet{}
        , tm(sched, taskSet, maxTasks)
        , numThreads(numThreads_)
    {
        printf("Box2DThreading constructor - maxTasks: %u, numThreads: %u\n", maxTasks, numThreads);
        TaskSchedulerConfig config{sched.GetConfig()};
        config.numTaskThreadsToCreate = numThreads - 1;  // -1 because main thread counts
        sched.Initialize(config);
        taskSet.Init(numThreads);  // Pass the full thread count to taskSet
        printf("Box2DThreading initialization complete - actual threads: %u\n", sched.GetNumTaskThreads());
    }
};

EMSCRIPTEN_BINDINGS(box2d_setup) {
    class_<Box2DThreading>("Box2DThreading")
        .constructor<uint32_t, uint32_t>()
        ;

	function("createThreadedWorld", optional_override([](
		b2WorldDef& worldDef,
		Box2DThreading& threading
	) -> b2WorldId {
		// (1) Hook the threading system as before
		printf("Hooking threading system to world definition\n");
		worldDef.enqueueTask = &TaskManager::StaticEnqueueTask;
		worldDef.finishTask = &TaskManager::StaticFinishTask;
		worldDef.userTaskContext = &threading.tm;
		worldDef.workerCount = threading.numThreads;
		printf("Threading hooked with %d workers\n", worldDef.workerCount);

		// (2) Actually create the b2World
		b2WorldId worldId = b2CreateWorld(&worldDef);

		// (3) Return its ID to JavaScript
		return worldId;
	}));
}

EMSCRIPTEN_BINDINGS(enki) {
	class_<ICompletable>("ICompletable")
        ;

    class_<ITaskSet, base<ICompletable>>("ITaskSet")
        .property("m_SetSize", &ITaskSet::m_SetSize)
        .property("m_MinRange", &ITaskSet::m_MinRange)
        ;

	class_<TaskSchedulerConfig>("TaskSchedulerConfig")
		.constructor()
		.property("numTaskThreadsToCreate", &TaskSchedulerConfig::numTaskThreadsToCreate)
		.property("numExternalTaskThreads", &TaskSchedulerConfig::numExternalTaskThreads)
		;

	class_<TaskScheduler>("TaskScheduler")
		.constructor()
		.function("Initialize", select_overload<void(TaskSchedulerConfig)>(&TaskScheduler::Initialize))
		.function("AddTaskSetToPipe", &TaskScheduler::AddTaskSetToPipe, allow_raw_pointers())
		.function("GetNumTaskThreads", &TaskScheduler::GetNumTaskThreads)
		;
}

EMSCRIPTEN_BINDINGS(threading) {
    class_<B2DTaskSet, base<ITaskSet>>("B2DTaskSet")
        .constructor()
        .constructor<uint32_t>()
        .constructor<uint32_t, uint32_t>()
        .function("Init", &B2DTaskSet::Init)
        ;

    class_<TaskManager>("TaskManager")
        .constructor<TaskScheduler&, B2DTaskSet&>()
        .constructor<TaskScheduler&, B2DTaskSet&, uint32_t>()
        .class_function("StaticEnqueueTask",
            optional_override([](
                emscripten::val callback,
                int itemCount,
                int minRange,
                uintptr_t box2dContext,
                uintptr_t userContext
            ) -> uintptr_t {
                auto fnPtr = reinterpret_cast<b2TaskCallback*>(
                    reinterpret_cast<void*>(callback.as<uintptr_t>())
                );
                return reinterpret_cast<uintptr_t>(
                    TaskManager::StaticEnqueueTask(
                        fnPtr,
                        itemCount,
                        minRange,
                        reinterpret_cast<void*>(box2dContext),
                        reinterpret_cast<void*>(userContext)
                    )
                );
            }))
        .class_function("StaticFinishTask",
            optional_override([](
                uintptr_t userTask,
                uintptr_t userContext
            ) {
                TaskManager::StaticFinishTask(
                    reinterpret_cast<void*>(userTask),
                    reinterpret_cast<void*>(userContext)
                );
            }))
        ;
}
