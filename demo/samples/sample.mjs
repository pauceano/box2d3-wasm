import settings, {DEFAULT_SETTINGS} from './settings.mjs';

export default class Sample{
	constructor(box2d){

		Object.assign(settings, DEFAULT_SETTINGS);

		this.box2d = box2d;
		const {
			b2DefaultWorldDef,
			b2CreateWorld,
			b2CreateThreadedWorld,
		} = box2d;

		this.worldDef = b2DefaultWorldDef();
		this.worldDef.enableSleep = settings.enableSleep;

		if(settings.workerCount > 1){
			this.m_taskSystem = new TaskSystem(settings.workerCount);
			this.m_worldId = b2CreateThreadedWorld(this.worldDef, this.m_taskSystem);
		} else {
			this.m_worldId = b2CreateWorld(this.worldDef);
		}

		this.pane = null;
	}

	Step(){
		const {
			b2World_EnableSleeping,
			b2World_EnableWarmStarting,
			b2World_EnableContinuous,
			b2World_Step,
		} = this.box2d;

		const timeStep = settings.hertz > 0.0 ? 1.0 / settings.hertz : 0.0;

		b2World_EnableSleeping( this.m_worldId, settings.enableSleep );
		b2World_EnableWarmStarting( this.m_worldId, settings.enableWarmStarting );
		b2World_EnableContinuous( this.m_worldId, settings.enableContinuous );

		for ( let i = 0; i < 1; ++i )
		{
			b2World_Step( this.m_worldId, timeStep, settings.subStepCount );
			this.m_taskSystem?.ClearTasks();
		}
	}

	Spawn(){}
	Despawn(){}
	CreateUI(){}
	UpdateUI(){}
	Destroy(){}
}
