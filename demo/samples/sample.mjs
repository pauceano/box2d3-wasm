<<<<<<< HEAD:demo/samples/sample.mjs
import Box2DDebugDraw from '../utils/debugDraw.mjs';
=======
import DebugDrawRenderer from '../utils/debugDraw.js';
>>>>>>> ef2f14d (improved debugDraw utility):demo/samples/sample.js

const settings = {
	workerCount: 1,
	enableSleep: true,
	// draw options
	drawShapes: true,
	drawJoints: false,
	drawJointExtras: false,
	drawAABBs: false,
	drawMass: false,
	drawContactPoints: false,
	drawGraphColors: false,
	drawContactNormals: false,
	drawContactImpulses: false,
	drawFrictionImpulses: false,
	// simulation options
	hertz: 60,
	enableWarmStarting: true,
	enableContinuous: true,
	subStepCount: 4,
}

const camera = {
	offset: { x: 11, y: -12 },
	ptm: 32,
};

export default class Sample{
	constructor(box2d, canvas){
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
		} = box2d;

		const timeStep = settings.hertz > 0.0 ? 1.0 / settings.hertz : 0.0;

		this.debugDraw.drawShapes = settings.drawShapes;
		this.debugDraw.drawJoints = settings.drawJoints;
		this.debugDraw.drawJointExtras = settings.drawJointExtras;
		this.debugDraw.drawAABBs = settings.drawAABBs;
		this.debugDraw.drawMass = settings.drawMass;
		this.debugDraw.drawContacts = settings.drawContactPoints;
		this.debugDraw.drawGraphColors = settings.drawGraphColors;
		this.debugDraw.drawContactNormals = settings.drawContactNormals;
		this.debugDraw.drawContactImpulses = settings.drawContactImpulses;
		this.debugDraw.drawFrictionImpulses = settings.drawFrictionImpulses;

		b2World_EnableSleeping( this.m_worldId, settings.enableSleep );
		b2World_EnableWarmStarting( this.m_worldId, settings.enableWarmStarting );
		b2World_EnableContinuous( this.m_worldId, settings.enableContinuous );

		for ( let i = 0; i < 1; ++i )
		{
			b2World_Step( this.m_worldId, timeStep, settings.subStepCount );
			this.m_taskSystem?.clearTasks();
		}

		
	}

	UpdateUI(){

	}

	Destroy(){

	}
}
