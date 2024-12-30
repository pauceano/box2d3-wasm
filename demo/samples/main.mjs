import {Pane} from 'https://cdn.jsdelivr.net/npm/tweakpane@4.0.5/dist/tweakpane.min.js';

import Box2DFactory from 'box2d3-wasm';
import settings from './settings.mjs';
import Camera from '../utils/camera.mjs';
import DebugDrawRenderer from '../utils/debugDraw.mjs';

const state = {
	pause: false,
	singleStep: false,
}

let box2d = null;
let sample = null;
let pane = null;

const canvas = document.getElementById("demo-canvas");
const ctx = canvas.getContext("2d");


const camera = new Camera({autoResize: true, controls: true, canvas});
let debugDraw = null;

function loadSample(url) {
	import(url).then((module) => {
		sample = new module.default(box2d, camera);
		updateDebugDrawFlags();
	});
}

const debugDrawFlagKeys = ['drawShapes', 'drawJoints', 'drawJointExtras', 'drawAABBs', 'drawMass', 'drawContactPoints', 'drawGraphColors', 'drawContactImpulses', 'drawContactNormals', 'drawContactImpulses', 'drawFrictionImpulses'];
function updateDebugDrawFlags(){
	const debugDrawFlags = {};
	debugDrawFlagKeys.forEach((key) => {
		debugDrawFlags[key] = settings[key];
	});
	debugDraw.SetFlags(debugDrawFlags);
}

async function initialize(){
	box2d = await Box2DFactory();

	debugDraw = new DebugDrawRenderer(box2d, ctx, settings.ptm);

	requestAnimationFrame(update);

	loadSample('./events/SensorFunnel.mjs');

	addUI();
}

function addUI(){
	const container = document.getElementById('main-settings');

	const PARAMS = {
		pause: false,
	  };
	pane = new Pane({
		title: 'Main Settings',
		expanded: true,
		container,
	});

	pane.addBinding(PARAMS, 'pause').on('change', (event) => {
		state.pause = event.value;
	});

	pane.addButton({
		title: 'single step',
	}).on('click', () => {
		state.singleStep = true;
	});

	// debug draw settings
	const debugDrawFolder = pane.addFolder({
		title: 'debugdraw',
		expanded: false,
	});
	debugDrawFlagKeys.forEach((key) => {
		debugDrawFolder.addBinding(settings, key).on('change', updateDebugDrawFlags);
	});
}

let lastFrameTime = 0;
let frame = 0;
let frameTime = 0;

function update(timestamp) {
    const deltaTime = timestamp - lastFrameTime;

    if (deltaTime >= settings.maxFrameTime && sample) {
		ctx.clearRect(0, 0, canvas.width, canvas.height);

        const start = performance.now();
		if (!state.pause || state.singleStep) {
        	sample.Step()
		}
        const end = performance.now();

		state.singleStep = false;

		debugDraw.Draw(sample.m_worldId, camera);

        frameTime = end - start;

        lastFrameTime = timestamp - (deltaTime % settings.maxFrameTime);
        frame++;
    }

    requestAnimationFrame(update);
}


initialize();
