
import Box2DFactory from 'box2d3-wasm';
import settings, {DEFAULT_SETTINGS} from './settings.mjs';

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
}

let lastFrameTime = 0;
let frame = 0;
let frameTime = 0;

function update(timestamp) {
    const deltaTime = timestamp - lastFrameTime;

    if (deltaTime >= settings.maxFrameTime) {
		ctx.clearRect(0, 0, canvas.width, canvas.height);

        const time1 = performance.now();
        sample?.Step()
        const time2 = performance.now();
        frameTime = time2 - time1;

        lastFrameTime = timestamp - (deltaTime % settings.maxFrameTime);
        frame++;
    }

    requestAnimationFrame(update);
}


initialize();
