import {Pane} from 'https://cdn.jsdelivr.net/npm/tweakpane@4.0.5/dist/tweakpane.min.js';

import Box2DFactory from 'box2d3-wasm';
import settings from './settings.mjs';

const state = {
	pause: false,
	singleStep: false,
}

let box2d = null;
let sample = null;

const canvas = document.getElementById("demo-canvas");
const ctx = canvas.getContext("2d");

function loadSample(url) {
	import(url).then((module) => {
		sample = new module.default(box2d, canvas);
	});
}


async function initialize(){
	box2d = await Box2DFactory();

	requestAnimationFrame(update);

	loadSample('./events/SensorFunnel.mjs');

	addUI();
}

function addUI(){
	const container = document.getElementById('main-settings');

	const PARAMS = {
		pause: false,
	  };
	const pane = new Pane({
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
}

let lastFrameTime = 0;
let frame = 0;
let frameTime = 0;

function update(timestamp) {
    const deltaTime = timestamp - lastFrameTime;

    if (deltaTime >= settings.maxFrameTime) {
		ctx.clearRect(0, 0, canvas.width, canvas.height);

        const start = performance.now();
		if (!state.pause || state.singleStep) {
        	sample?.Step()
		}
        const end = performance.now();

		state.singleStep = false;

		sample?.Draw();

        frameTime = end - start;

        lastFrameTime = timestamp - (deltaTime % settings.maxFrameTime);
        frame++;
    }

    requestAnimationFrame(update);
}


initialize();
