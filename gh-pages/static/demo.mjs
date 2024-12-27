
import DebugDrawRenderer from './debugDraw.mjs';
import Box2DFactory from 'box2d3-wasm';

const box2d = await Box2DFactory();
const canvas = document.getElementById("demo-canvas");
const ctx = canvas.getContext("2d");

const pixelsPerMeter = 10;
const subStepCount = 4;

const debugDraw = new DebugDrawRenderer(box2d, ctx, pixelsPerMeter);
debugDraw.offset = {
  x: 40,
  y: -29
};


const {
  b2DefaultWorldDef,
  b2CreateWorld,
  b2CreateBody,
  b2CreatePolygonShape,
  b2CreateCircleShape,
  b2CreateSegmentShape,
  b2World_Step,
  b2MakeBox,
  b2Circle,
  b2DefaultBodyDef,
  b2DefaultShapeDef,
  b2BodyType,
  b2Segment,
  b2Vec2,
  b2Rot,
  b2Body_SetTransform,
  b2Body_SetLinearVelocity,
  b2Body_SetAwake,
  b2Body_Enable,
  Sample,
  createThreadedSampleWorld,
  b2World_GetProfile,
  CanvasDebugDraw,
  b2World_Draw
} = box2d;


const canvasDebugDraw = new CanvasDebugDraw(6000);


const worldDef = b2DefaultWorldDef();
worldDef.gravity.Set(0, -10);

let worldId, sample;

const params = new URLSearchParams(window.location.search);

const statsLevel = params.get('stats') || 2;

if(params.get('threading') === '1') {
  sample = new Sample(navigator.hardwareConcurrency);
  worldId = createThreadedSampleWorld(worldDef, sample);
} else {
  worldId = b2CreateWorld(worldDef);
}

const bd_ground = new b2DefaultBodyDef();
const groundId = b2CreateBody(worldId, bd_ground);

const shapeDefSegment = new b2DefaultShapeDef();
shapeDefSegment.density = 1.0;
shapeDefSegment.friction = 0.3;

{
  const segment = new b2Segment();
  segment.point1 = new b2Vec2(3, -4);
  segment.point2 = new b2Vec2(6, -7);

  b2CreateSegmentShape(groundId, shapeDefSegment, segment);
}

{
  const segment = new b2Segment();
  segment.point1 = new b2Vec2(3, -18);
  segment.point2 = new b2Vec2(22, -18)
  b2CreateSegmentShape(groundId, shapeDefSegment, segment);
}

{
  const segment = new b2Segment();
  segment.point1 = new b2Vec2(-100, -40);
  segment.point2 = new b2Vec2(100, -40)
  b2CreateSegmentShape(groundId, shapeDefSegment, segment);
}

const sideLengthMetres = 1;

const ZERO = new b2Vec2(0, 0);
const temp = new b2Vec2(0, 0);

const rot = new b2Rot();
rot.SetAngle(0);

const initPosition = (bodyId, index) => {
  console.log('initPosition', bodyId, index);
  temp.Set(4 + sideLengthMetres*(Math.random()-0.5), sideLengthMetres*index);
  b2Body_SetTransform(bodyId, temp, rot);
  b2Body_SetLinearVelocity(bodyId, ZERO);
  b2Body_SetAwake(bodyId, 1);
  b2Body_Enable(bodyId);
}



// const boxCount = 10;
// for (let i = boxCount; i >= 0; i--) {

//   const bd = new b2DefaultBodyDef();
//   bd.type = b2BodyType.b2_dynamicBody;
//   bd.position = ZERO;

//   const bodyId = b2CreateBody(worldId, bd);

//   console.log('bodyId', bodyId);

//   const shapeDefDynamic = new b2DefaultShapeDef();
//   shapeDefDynamic.density = 1.0;
//   shapeDefDynamic.friction = 0.3;

//   const square = b2MakeBox(sideLengthMetres/2, sideLengthMetres/2);
//   const circle = new b2Circle();
//   circle.radius = sideLengthMetres/2;

//   i % 2 ? b2CreatePolygonShape(bodyId, shapeDefDynamic, square) : b2CreateCircleShape(bodyId, shapeDefDynamic, circle);

//   initPosition(bodyId, i);
// }


  function createPyramid(worldId, height, gap) {
    const boxWidth = 1.0;
    const boxHeight = 1.0;

    for (let row = 0; row < height; row++) {
        const boxesInRow = height - row;

        const startX = -(boxesInRow - 1) * (boxWidth + gap) / 2;

        for (let i = 0; i < boxesInRow; i++) {
            const bd = new b2DefaultBodyDef();
            bd.type = b2BodyType.b2_dynamicBody;

            const xPos = startX + i * (boxWidth + gap);
            const yPos = (boxHeight + gap) * row + boxHeight;
            bd.position = new b2Vec2().Set(xPos, yPos);

            const bodyId = b2CreateBody(worldId, bd);

            const shapeDefDynamic = new b2DefaultShapeDef();
            shapeDefDynamic.density = 1.0;
            shapeDefDynamic.friction = 0.3;

            const box = b2MakeBox(boxWidth/2, boxHeight/2);
            b2CreatePolygonShape(bodyId, shapeDefDynamic, box);
        }
    }
}

const pyramidHeight = params.get('pyramidHeight') ? parseInt(params.get('pyramidHeight')) : 10;
const boxGap = 0.1;
createPyramid(worldId, pyramidHeight, boxGap);

/*
  b2Timer timer = b2CreateTimer();

      for ( int step = 1; step < stepCount; ++step )
      {
        if ( benchmark->stepFcn != NULL)
        {
          benchmark->stepFcn( worldId, step );
        }
        b2World_Step( worldId, timeStep, subStepCount );
        taskCount = 0;
      }

      float ms = b2GetMilliseconds( &timer );
      float fps = 1000.0f * stepCount / ms;
      printf( "run %d : %g (ms), %g (fps)\n", runIndex, ms, fps );
*/


function drawProfile(stepDuration, profile) {
  ctx.font = "16px Arial";
  ctx.fillStyle = "black";
  if (statsLevel < 1) return;
  ctx.fillText(`fps: ${Math.floor(1000/stepDuration)}`, 10, 20);
  ctx.fillText(`threading: ${sample ? 'on' : 'off'}`, 100, 20);
  ctx.fillText(`memory: ${performance.memory?.usedJSHeapSize ?? '(Unavailable)'}`, 300, 20);
  if (statsLevel < 2) return;
  ctx.fillText(`step: ${profile.step.toFixed(2)}ms`, 10, 40);
  ctx.fillText(`pairs: ${profile.pairs.toFixed(2)}ms`, 10, 60);
  ctx.fillText(`collide: ${profile.collide.toFixed(2)}ms`, 10, 80);
  ctx.fillText(`solve: ${profile.solve.toFixed(2)}ms`, 10, 100);
  ctx.fillText(`buildIslands: ${profile.buildIslands.toFixed(2)}ms`, 10, 120);
  ctx.fillText(`solveConstraints: ${profile.solveConstraints.toFixed(2)}ms`, 10, 140);
  ctx.fillText(`prepareTasks: ${profile.prepareTasks.toFixed(2)}ms`, 10, 160);
  ctx.fillText(`solverTasks: ${profile.solverTasks.toFixed(2)}ms`, 10, 180);
  ctx.fillText(`prepareConstraints: ${profile.prepareConstraints.toFixed(2)}ms`, 10, 200);
  ctx.fillText(`integrateVelocities: ${profile.integrateVelocities.toFixed(2)}ms`, 10, 220);
  ctx.fillText(`warmStart: ${profile.warmStart.toFixed(2)}ms`, 10, 240);
  ctx.fillText(`solveVelocities: ${profile.solveVelocities.toFixed(2)}ms`, 10, 260);
  ctx.fillText(`integratePositions: ${profile.integratePositions.toFixed(2)}ms`, 10, 280);
  ctx.fillText(`relaxVelocities: ${profile.relaxVelocities.toFixed(2)}ms`, 10, 300);
  ctx.fillText(`applyRestitution: ${profile.applyRestitution.toFixed(2)}ms`, 10, 320);
  ctx.fillText(`storeImpulses: ${profile.storeImpulses.toFixed(2)}ms`, 10, 340);
  ctx.fillText(`finalizeBodies: ${profile.finalizeBodies.toFixed(2)}ms`, 10, 360);
  ctx.fillText(`sleepIslands: ${profile.sleepIslands.toFixed(2)}ms`, 10, 380);
  ctx.fillText(`splitIslands: ${profile.splitIslands.toFixed(2)}ms`, 10, 400);
  ctx.fillText(`hitEvents: ${profile.hitEvents.toFixed(2)}ms`, 10, 420);
  ctx.fillText(`broadphase: ${profile.broadphase.toFixed(2)}ms`, 10, 440);
  ctx.fillText(`continuous: ${profile.continuous.toFixed(2)}ms`, 10, 460);
}

let handle;
function loop(prevMs) {
    const nowMs = window.performance.now();
    handle = requestAnimationFrame(loop.bind(null, nowMs));
    const deltaMs = nowMs-prevMs;

    const start = performance.now();
    b2World_Step(worldId, deltaMs / 1000, subStepCount);
    const end = performance.now();
    sample?.resetTaskCount();

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    b2World_Draw(worldId, canvasDebugDraw.GetDebugDraw());

    const commandsPtr = canvasDebugDraw.GetCommandsData();
    const commandsSize = canvasDebugDraw.GetCommandsSize();
    const commandStride = canvasDebugDraw.GetCommandStride();
    debugDraw.processCommands(commandsPtr, commandsSize, commandStride);
    canvasDebugDraw.ClearCommands();

    const duration = end - start;
    const profile = b2World_GetProfile(worldId);
    drawProfile(duration, profile);
};

loop(window.performance.now());
