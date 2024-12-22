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
	struct TaskData
	{
		b2TaskCallback* box2dTask = nullptr;
		void* box2dContext = nullptr;
	};
	private:
	std::vector<TaskData> data;
	public:
	void Init(uint32_t num_threads)
    {
		EnsureSize(num_threads);
	}
	void EnsureSize(uint32_t num_threads)
    {
		data.resize(num_threads);
	}
	void Assign(uint32_t index, TaskData taskData)
    {
		data[index] = taskData;
	}
    void ExecuteRange(TaskSetPartition range_, uint32_t threadnum_ ) override {
		const TaskData& taskData{data[threadnum_]};
		taskData.box2dTask(range_.start, range_.end, threadnum_, taskData.box2dContext);
    }
};

struct TaskManager {
	// you should Initialize your sched first
	TaskScheduler& sched;
	// you can optionally Init taskSet to the right number of threads to avoid resizing its vector multiple times
	B2DTaskSet& taskSet;
	uint32_t maxTasks;
	uint32_t taskCount = 0;
	TaskManager(TaskScheduler& sched, B2DTaskSet& taskSet, uint32_t maxTasks = 128)
		: sched(sched)
		, taskSet(taskSet)
		, maxTasks(maxTasks)
	{}
	void* EnqueueTask( b2TaskCallback* box2dTask, int itemCount, int minRange, void* box2dContext, [[maybe_unused]] void* userContext )
	{
		if (taskCount >= maxTasks) {
			box2dTask( 0, itemCount, 0, box2dContext );
			return NULL;
		}
		taskSet.EnsureSize(taskCount);
		taskSet.Assign(taskCount, B2DTaskSet::TaskData{ .box2dTask = box2dTask, .box2dContext=box2dContext });

		taskSet.m_MinRange = minRange;
		taskSet.m_SetSize = itemCount;

		++taskCount;

		return &taskSet;
	}
	void FinishTask( void* userTask, [[maybe_unused]] void* userContext )
	{
		sched.WaitforTaskSet(reinterpret_cast<B2DTaskSet*>(userTask));
	}

	static void* StaticEnqueueTask(b2TaskCallback* box2dTask, int itemCount, int minRange, void* box2dContext, void* userContext) {
		TaskManager* tm{reinterpret_cast<TaskManager*>(userContext)};
        return tm->EnqueueTask(box2dTask, itemCount, minRange, box2dContext, userContext);
    }

    static void StaticFinishTask(void* userTask, void* userContext) {
		TaskManager* tm{reinterpret_cast<TaskManager*>(userContext)};
        tm->FinishTask(userTask, userContext);
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

EMSCRIPTEN_BINDINGS(enki) {
	class_<TaskSchedulerConfig>("TaskSchedulerConfig")
		.constructor()
		.property("numTaskThreadsToCreate", &TaskSchedulerConfig::numTaskThreadsToCreate)
		.property("numExternalTaskThreads", &TaskSchedulerConfig::numExternalTaskThreads)
		;

	class_<TaskScheduler>("TaskScheduler")
		.constructor()
		.function("Initialize", select_overload<void(TaskSchedulerConfig)>(&TaskScheduler::Initialize))
		.function("AddTaskSetToPipe", &TaskScheduler::AddTaskSetToPipe)
		;
}

EMSCRIPTEN_BINDINGS(threading) {
	class_<B2DTaskSet>("B2DTaskSet")
		.constructor()
		.constructor<uint32_t>()
		.constructor<uint32_t, uint32_t>()
		.function("Init", &B2DTaskSet::Init)
		.function("EnsureSize", &B2DTaskSet::EnsureSize)
		.function("Assign", &B2DTaskSet::Assign)
		;

	class_<B2DTaskSet>("TaskManager")
		.constructor<TaskScheduler&, B2DTaskSet&>()
		.constructor<TaskScheduler&, B2DTaskSet&, uint32_t>()
        .function("EnqueueTask", &TaskManager::EnqueueTask, allow_raw_pointers())
        .function("FinishTask", &TaskManager::FinishTask, allow_raw_pointers())
        .class_function("StaticEnqueueTask", &TaskManager::StaticEnqueueTask, allow_raw_pointers())
        .class_function("StaticFinishTask", &TaskManager::StaticFinishTask, allow_raw_pointers())
		;

}