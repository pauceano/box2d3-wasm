import { strict as assert } from 'assert';
import Box2DFactory from '../build/dist/es/entry.mjs'
const box2d = await Box2DFactory()
const {
    b2DefaultWorldDef,
    b2WorldDef,
    World,
} = box2d;
const B2_SECRET_COOKIE = 1152023
const worldDef = new b2WorldDef(b2DefaultWorldDef())
assert(worldDef.internalValue === B2_SECRET_COOKIE)
const world = new World(worldDef)
console.log('hello', world)