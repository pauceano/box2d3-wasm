
import DebugDrawRenderer, {Camera} from '../utils/debugDraw.mjs';

import settings, {DEFAULT_SETTINGS} from './settings.mjs';

export default class Sample{
	constructor(box2d, canvas){

		Object.assign(settings, DEFAULT_SETTINGS);

		this.camera = new Camera();
		this.box2d = box2d;
		this.canvas = canvas;
		this.ctx = canvas.getContext('2d');
		const {
			b2DefaultWorldDef,
			b2CreateWorld,
			b2CreateThreadedWorld,
		} = box2d;

		this.worldDef = b2DefaultWorldDef();
		this.worldDef.enableSleep = settings.enableSleep;

		if(settings.workerCount > 1){
			this.m_taskSystem = new TaskSystem(navigator.hardwareConcurrency);
			this.m_worldId = b2CreateThreadedWorld(this.worldDef, this.m_taskSystem);
		} else {
			this.m_worldId = b2CreateWorld(this.worldDef);
		}

		this.debugDraw = new DebugDrawRenderer(box2d, this.ctx, settings.ptm);
	}

	Step(){
		const {
			b2World_EnableSleeping,
			b2World_EnableWarmStarting,
			b2World_EnableContinuous,
			b2World_Step,
		} = this.box2d;

		const timeStep = settings.hertz > 0.0 ? 1.0 / settings.hertz : 0.0;

		this.debugDraw.SetFlags({
			drawShapes: settings.drawShapes,
			drawJoints: settings.drawJoints,
			drawJointExtras: settings.drawJointExtras,
			drawAABBs: settings.drawAABBs,
			drawMass: settings.drawMass,
			drawContacts: settings.drawContactPoints,
			drawGraphColors: settings.drawGraphColors,
			drawContactNormals: settings.drawContactNormals,
			drawContactImpulses: settings.drawContactImpulses,
			drawFrictionImpulses: settings.drawFrictionImpulses,
		});

		b2World_EnableSleeping( this.m_worldId, settings.enableSleep );
		b2World_EnableWarmStarting( this.m_worldId, settings.enableWarmStarting );
		b2World_EnableContinuous( this.m_worldId, settings.enableContinuous );

		for ( let i = 0; i < 1; ++i )
		{
			b2World_Step( this.m_worldId, timeStep, settings.subStepCount );
			this.m_taskSystem?.ClearTasks();
		}

		this.debugDraw.Draw(this.m_worldId, this.camera);
	}

	UpdateUI(){

	}

	Destroy(){

	}
}
