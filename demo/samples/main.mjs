import {Pane} from 'https://cdn.jsdelivr.net/npm/tweakpane@4.0.5/dist/tweakpane.min.js';

import Box2DFactory from 'box2d3-wasm';
import Camera from '../utils/camera.mjs';
import DebugDrawRenderer from '../utils/debugDraw.mjs';
import Keyboard, { Key } from '../utils/keyboard.mjs';

import samples from './categories/list.mjs';
import settings from './settings.mjs';

const state = {
	mouseDown: false,
	mousePos: {x: 0, y: 0},
	mouseDownPos: {x: 0, y: 0},
	mouseJoint: null,
}

let box2d = null;
let sample = null;
let sampleUrl = './categories/events/sensorFunnel.mjs';
let sampleName = null;
let pane = null;

const params = new URLSearchParams(window.location.search);
if(params.has('sample')){
	const sampleName = params.get('sample');
	const paths = Object.values(samples).flatMap(category => Object.values(category));
	const sample = paths.find((url) => url.includes(sampleName));
	if(sample){
		sampleUrl = sample;
	}
}

const canvas = document.getElementById("demo-canvas");
const ctx = canvas.getContext("2d");

const camera = new Camera({autoResize: true, controls: true, canvas});
let debugDraw = null;

function loadSample(url) {
	console.log('loading sample', url);
	if(sample){
		sample.Destroy();
		sample = null;
	}

	sampleUrl = url;
	sampleName = url.slice(13);
	window.history.pushState({}, sampleName, `?sample=${sampleName}`);

	import(url).then((module) => {
		sample = new module.default(box2d, camera, debugDraw);
		updateDebugDrawFlags();
	});
}

const debugDrawFlagKeys = ['drawShapes', 'drawJoints', 'drawJointExtras', 'drawAABBs', 'drawMass', 'drawContacts', 'drawGraphColors', 'drawContactNormals', 'drawContactImpulses', 'drawFrictionImpulses'];
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

	Keyboard.Init();

	loadSample(sampleUrl);

	addUI();
	addControls();
}

function addUI(){
	const container = document.getElementById('main-settings');

	const PARAMS = {
		pause: false,
		'warm starting': settings.enableWarmStarting,
		continuous: settings.enableContinuous,
	  };
	pane = new Pane({
		title: 'Main Settings',
		expanded: true,
		container,
	});


	const tab = pane.addTab({
		pages: [
			{title: 'Controls'},
			{title: 'Samples'},
		],
	});

	const main = tab.pages[0];

	main.addBinding(PARAMS, 'pause').on('change', (event) => {
		settings.pause = event.value;
	});

	main.addBinding(PARAMS, 'warm starting').on('change', (event) => {
		settings.enableWarmStarting = event.value;
	});

	main.addBinding(PARAMS, 'continuous').on('change', (event) => {
		settings.enableContinuous = event.value;
	});

	main.addButton({
		title: 'single step',
	}).on('click', () => {
		settings.singleStep = true;
	});

	// debug draw settings
	const debugDrawFolder = main.addFolder({
		title: 'debugdraw',
		expanded: false,
	});
	debugDrawFlagKeys.forEach((key) => {
		debugDrawFolder.addBinding(settings, key).on('change', updateDebugDrawFlags);
	});

	const samplesTab = tab.pages[1];
	Object.keys(samples).forEach((type) => {
		// add folder for each sample
		const folder = samplesTab.addFolder({
			title: type,
			expanded: false,
		});

		Object.keys(samples[type]).forEach((sample) => {
			const url = samples[type][sample];
			folder.addButton({
				title: sample,
			}).on('click', () => {
				loadSample(url);
			});
		});
	});
}

function onPointerDown(event){
	state.mouseDown = true;
	state.mousePos = {x: event.clientX, y: event.clientY};
	state.mouseDownPos = {x: event.clientX, y: event.clientY};
	const idleClickTime = 100;
	setTimeout(() => {
		const maxMovementForClick = 10;
		if (
			state.mouseDown
			&& Math.abs(state.mousePos.x - state.mouseDownPos.x) < maxMovementForClick
			&& Math.abs(state.mousePos.y - state.mouseDownPos.y) < maxMovementForClick
		) {
			const p = camera.convertScreenToWorld(state.mousePos);
			const worldPos = new box2d.b2Vec2().Set(p.x, p.y);

			const interacting = sample?.MouseDown(worldPos);
			// block camera controls if interacting with the sample
			canvas.blockTouchCameraControls = interacting;
		}
	}, idleClickTime);
}

function onPointerMove(event){
	state.mousePos = {x: event.clientX, y: event.clientY};
	const p = camera.convertScreenToWorld(state.mousePos);
	const worldPos = new box2d.b2Vec2().Set(p.x, p.y);
	sample?.MouseMove(worldPos);
}

function onPointerUp(event){
	state.mouseDown = false;
	delete canvas.blockTouchCameraControls;
	sample?.MouseUp();
}

function addControls(){
	canvas.addEventListener('pointerdown', onPointerDown);
	canvas.addEventListener('pointerup', onPointerUp);
	canvas.addEventListener('pointermove', onPointerMove);
}

let lastFrameTime = 0;
let frame = 0;
let frameTime = 0;

let m_textLine = 0;

function DrawString(x, y, text){
	const fontHeight = 14 * Math.min(window.devicePixelRatio || 1, 2);
	ctx.font = `${fontHeight}px Arial`;
	ctx.fillStyle = 'rgba(230, 153, 153, 1)';
	ctx.fillText(text, x, y + fontHeight);
	const linePadding = 2;
	m_textLine += fontHeight + linePadding;
	return m_textLine;
}

function update(timestamp) {
    const deltaTime = timestamp - lastFrameTime;

	m_textLine = 0;

	if(Keyboard.IsPressed(Key.R)){
		loadSample(sampleUrl);
	}

	if(Keyboard.IsPressed(Key.P)){
		settings.pause = !settings.pause;
	}

    if (deltaTime >= settings.maxFrameTime) {
		ctx.clearRect(0, 0, canvas.width, canvas.height);

		if(sample){
			const start = performance.now();
			sample.Step()
			const end = performance.now();

			DrawString(5, m_textLine, sampleName);
			sample?.UpdateUI(DrawString, m_textLine);

			frameTime = end - start;

			lastFrameTime = timestamp - (deltaTime % settings.maxFrameTime);
			frame++;
		}

		Keyboard.Update();
    }

    requestAnimationFrame(update);
}


initialize();
