import Box2DFactory from '../build/dist/es/entry.mjs'
const box2d = await Box2DFactory()
const {
    b2WorldDef,
    World,
} = box2d;
const worldDef = new b2WorldDef()
const world = new World(worldDef)
console.log('hey')