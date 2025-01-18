#include <box2d/box2d.h>
#include <box2d/math_functions.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace b2;

template<typename T>
struct GetInterfaceType;

template<>
struct GetInterfaceType<Body> {
    using type = BasicBodyInterface<Body, false>;
};

template<>
struct GetInterfaceType<Shape> {
    using type = BasicShapeInterface<Shape, false>;
};

template<>
struct GetInterfaceType<Joint> {
    using type = BasicJointInterface<Joint, false>;
};

template<typename T, typename ObjectType>
emscripten::val getArrayWrapper(const ObjectType& object,
    int (GetInterfaceType<ObjectType>::type::*getCount)() const,
    int (GetInterfaceType<ObjectType>::type::*getData)(T*, int) const) {

    int capacity = (object.*getCount)();
    if (capacity == 0) return emscripten::val::array();

    std::vector<T> items(capacity);
    int count = (object.*getData)(items.data(), capacity);

    auto result = emscripten::val::array();
    for (int i = 0; i < count; i++) {
        result.set(i, items[i]);
    }
    return result;
}

template<typename T>
emscripten::val getEventsArray(T* events, int count) {
    if (count == 0) return emscripten::val::array();
    auto result = emscripten::val::array();
    for (int i = 0; i < count; i++) {
        result.set(i, events[i]);
    }
    return result;
}

template<typename T>
emscripten::val toBytes(const T& value) {
    auto array = emscripten::val::global("Uint8Array").new_(sizeof(T));
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    for (size_t i = 0; i < sizeof(T); i++) {
        array.set(i, bytes[i]);
    }
    return array;
}

EMSCRIPTEN_BINDINGS(box2dcpp) {
    class_<b2Vec2>("b2Vec2")
        .constructor()
        .constructor(+[](float x, float y) -> b2Vec2 { return b2Vec2{x, y}; })
        .property("x", &b2Vec2::x)
        .property("y", &b2Vec2::y)
        .function("Set", +[](b2Vec2& self, float x, float y) -> b2Vec2& {
            self.x = x;
            self.y = y;
            return self;
        }, return_value_policy::reference())
        .function("Copy", +[](b2Vec2& self, const b2Vec2& other) -> b2Vec2& {
            self.x = other.x;
            self.y = other.y;
            return self;
        }, return_value_policy::reference())
        .function("Clone", +[](const b2Vec2& self) -> b2Vec2 {
            return b2Vec2{self.x, self.y};
        })
        .function("Add", +[](b2Vec2& self, const b2Vec2& other) -> b2Vec2& {
            self.x += other.x;
            self.y += other.y;
            return self;
        }, return_value_policy::reference())
        .function("Sub", +[](b2Vec2& self, const b2Vec2& other) -> b2Vec2& {
            self.x -= other.x;
            self.y -= other.y;
            return self;
        }, return_value_policy::reference())
        .function("Mul", +[](b2Vec2& self, const b2Vec2& other) -> b2Vec2& {
            self.x *= other.x;
            self.y *= other.y;
            return self;
        }, return_value_policy::reference())
        .function("MulSV", +[](b2Vec2& self, float s) -> b2Vec2& {
            self.x *= s;
            self.y *= s;
            return self;
        }, return_value_policy::reference())
        ;
    constant("b2Vec2_zero", b2Vec2_zero);

    class_<b2CosSin>("b2CosSin")
        .constructor()
        .property("cosine", &b2CosSin::cosine)
        .property("sine", &b2CosSin::sine)
        ;

    class_<b2Rot>("b2Rot")
        .constructor()
        .property("c", &b2Rot::c)
        .property("s", &b2Rot::s)
        .function("SetAngle", +[](b2Rot& self, float angle) -> b2Rot {
            self.s = sinf(angle);
            self.c = cosf(angle);
            return self;
        })
        .function("GetAngle", +[](const b2Rot& self) -> float {
            return atan2f(self.s, self.c);
        })
        ;

    constant("b2Rot_identity", b2Rot_identity);

    class_<b2Transform>("b2Transform")
        .constructor()
        .property("p", &b2Transform::p, return_value_policy::reference())
        .property("q", &b2Transform::q, return_value_policy::reference())
        .function("ToBytes", +[](const b2Transform& self) -> emscripten::val {
            return toBytes(self);
        });
        ;

    class_<b2Mat22>("b2Mat22")
        .constructor()
        .property("cx", &b2Mat22::cx, return_value_policy::reference())
        .property("cy", &b2Mat22::cy, return_value_policy::reference())
        ;

    class_<b2AABB>("b2AABB")
        .constructor()
        .property("lowerBound", &b2AABB::lowerBound, return_value_policy::reference())
        .property("upperBound", &b2AABB::upperBound, return_value_policy::reference())
        ;

    class_<b2Hull>("b2Hull")
    .constructor()
    .property("count", &b2Hull::count)
    .function("getPoint", optional_override([](const b2Hull& hull, int index) {
        if (index >= 0 && index < hull.count) {
            return hull.points[index];
        }
        return b2Vec2();
    }))
    ;

    enum_<b2MixingRule>("b2MixingRule")
        .value("b2_mixAverage", b2MixingRule::b2_mixAverage)
        .value("b2_mixGeometricMean", b2MixingRule::b2_mixGeometricMean)
        .value("b2_mixMultiply", b2MixingRule::b2_mixMultiply)
        .value("b2_mixMinimum", b2MixingRule::b2_mixMinimum)
        .value("b2_mixMaximum", b2MixingRule::b2_mixMaximum)
        ;

    class_<b2WorldDef>("b2WorldDef")
        .constructor()
        .constructor<const b2WorldDef&>()
        .property("gravity", &b2WorldDef::gravity, return_value_policy::reference())
        .property("restitutionThreshold", &b2WorldDef::restitutionThreshold)
        .property("contactPushSpeed", &b2WorldDef::contactPushSpeed)
        .property("hitEventThreshold", &b2WorldDef::hitEventThreshold)
        .property("contactHertz", &b2WorldDef::contactHertz)
        .property("contactDampingRatio", &b2WorldDef::contactDampingRatio)
        .property("jointHertz", &b2WorldDef::jointHertz)
        .property("jointDampingRatio", &b2WorldDef::jointDampingRatio)
        .property("maximumLinearSpeed", &b2WorldDef::maximumLinearSpeed)
        .property("frictionMixingRule", &b2WorldDef::frictionMixingRule)
        .property("restitutionMixingRule", &b2WorldDef::restitutionMixingRule)
        .property("enableSleep", &b2WorldDef::enableSleep)
        .property("enableContinuous", &b2WorldDef::enableContinuous)
        .property("workerCount", &b2WorldDef::workerCount)
        // we do not assign threading callbacks here, we leave this in the C++ code
        .property("internalValue", &b2WorldDef::internalValue)
        ;

    class_<b2BodyId>("b2BodyId")
        .constructor()
        .property("index1", &b2BodyId::index1)
        .property("world0", &b2BodyId::world0)
        .property("revision", &b2BodyId::revision)
        ;


    class_<b2ShapeId>("b2ShapeId")
        .constructor()
        .property("index1", &b2ShapeId::index1)
        .property("world0", &b2ShapeId::world0)
        .property("revision", &b2ShapeId::revision)
        ;

    class_<b2WorldId>("b2WorldId")
        .constructor()
        .property("index1", &b2WorldId::index1)
        .property("revision", &b2WorldId::revision)
        ;

    class_<b2CastOutput>("b2CastOutput")
        .constructor()
        .property("normal", &b2CastOutput::normal, return_value_policy::reference())
        .property("point", &b2CastOutput::point, return_value_policy::reference())
        .property("fraction", &b2CastOutput::fraction)
        .property("iterations", &b2CastOutput::iterations)
        .property("hit", &b2CastOutput::hit)
        ;

    class_<b2RayCastInput>("b2RayCastInput")
        .constructor()
        .property("origin", &b2RayCastInput::origin, return_value_policy::reference())
        .property("translation", &b2RayCastInput::translation, return_value_policy::reference())
        .property("maxFraction", &b2RayCastInput::maxFraction)
        ;

    class_<b2ManifoldPoint>("b2ManifoldPoint")
        .constructor()
        .property("point", &b2ManifoldPoint::point, return_value_policy::reference())
        .property("anchorA", &b2ManifoldPoint::anchorA, return_value_policy::reference())
        .property("anchorB", &b2ManifoldPoint::anchorB, return_value_policy::reference())
        .property("separation", &b2ManifoldPoint::separation)
        .property("normalImpulse", &b2ManifoldPoint::normalImpulse)
        .property("tangentImpulse", &b2ManifoldPoint::tangentImpulse)
        .property("maxNormalImpulse", &b2ManifoldPoint::maxNormalImpulse)
        .property("normalVelocity", &b2ManifoldPoint::normalVelocity)
        .property("id", &b2ManifoldPoint::id)
        .property("persisted", &b2ManifoldPoint::persisted)
        ;

    class_<b2Manifold>("b2Manifold")
        .constructor()
        .function("GetPoint", +[](const b2Manifold& self, int index) -> b2ManifoldPoint {
            return self.points[index];
        })
        .function("SetPoint", +[](b2Manifold& self, int index, const b2ManifoldPoint& point) {
            self.points[index] = point;
        })
        .property("normal", &b2Manifold::normal, return_value_policy::reference())
        .property("pointCount", &b2Manifold::pointCount)
        ;

    class_<b2Counters>("b2Counters")
        .constructor()
        .property("bodyCount", &b2Counters::bodyCount)
        .property("shapeCount", &b2Counters::shapeCount)
        .property("contactCount", &b2Counters::contactCount)
        .property("jointCount", &b2Counters::jointCount)
        .property("islandCount", &b2Counters::islandCount)
        .property("stackUsed", &b2Counters::stackUsed)
        .property("staticTreeHeight", &b2Counters::staticTreeHeight)
        .property("treeHeight", &b2Counters::treeHeight)
        .property("byteCount", &b2Counters::byteCount)
        .property("taskCount", &b2Counters::taskCount)
        .function("GetColorCount", +[](const b2Counters& self, int index) -> int {
            if (index < 0 || index >= 12) {
                return 0;  // Return 0 for out of bounds
            }
            return self.colorCounts[index];
        })
        .function("SetColorCount", +[](b2Counters& self, int index, int value) {
            if (index >= 0 && index < 12) {
                self.colorCounts[index] = value;
            }
        })
        ;

    class_<b2SensorBeginTouchEvent>("b2SensorBeginTouchEvent")
        .constructor()
        .property("sensorShapeId", &b2SensorBeginTouchEvent::sensorShapeId)
        .property("visitorShapeId", &b2SensorBeginTouchEvent::visitorShapeId)
        ;

    class_<b2SensorEndTouchEvent>("b2SensorEndTouchEvent")
        .constructor()
        .property("sensorShapeId", &b2SensorEndTouchEvent::sensorShapeId)
        .property("visitorShapeId", &b2SensorEndTouchEvent::visitorShapeId)
        ;

    class_<b2SensorEvents>("b2SensorEvents")
        .constructor()
        .function("GetBeginEvents", +[](const b2SensorEvents& events) {
            return getEventsArray(events.beginEvents, events.beginCount);
        })
        .function("GetEndEvents", +[](const b2SensorEvents& events) {
            return getEventsArray(events.endEvents, events.endCount);
        })
        .property("beginCount", &b2SensorEvents::beginCount)
        .property("endCount", &b2SensorEvents::endCount)
        ;

    class_<b2BodyEvents>("b2BodyEvents")
        .constructor()
        .function("GetMoveEvents", +[](const b2BodyEvents& events) {
            return getEventsArray(events.moveEvents, events.moveCount);
        })
        .property("moveCount", &b2BodyEvents::moveCount)
        ;

    class_<b2BodyMoveEvent>("b2BodyMoveEvent")
        .constructor()
        .property("transform", &b2BodyMoveEvent::transform, return_value_policy::reference())
        .property("bodyId", &b2BodyMoveEvent::bodyId)
        .property("fellAsleep", &b2BodyMoveEvent::fellAsleep)
        ;

     class_<b2ContactEvents>("b2ContactEvents")
        .constructor()
        .property("beginCount", &b2ContactEvents::beginCount)
        .property("endCount", &b2ContactEvents::endCount)
        .property("hitCount", &b2ContactEvents::hitCount)
        .function("GetBeginEvents", +[](const b2ContactEvents& events) {
            return getEventsArray(events.beginEvents, events.beginCount);
        })
        .function("GetEndEvents", +[](const b2ContactEvents& events) {
            return getEventsArray(events.endEvents, events.endCount);
        })
        .function("GetHitEvents", +[](const b2ContactEvents& events) {
            return getEventsArray(events.hitEvents, events.hitCount);
        })
        ;

    class_<b2ContactBeginTouchEvent>("b2ContactBeginTouchEvent")
        .constructor()
        .property("shapeIdA", &b2ContactBeginTouchEvent::shapeIdA)
        .property("shapeIdB", &b2ContactBeginTouchEvent::shapeIdB)
        .property("manifold", &b2ContactBeginTouchEvent::manifold, return_value_policy::reference())
        ;

    class_<b2ContactData>("b2ContactData")
        .constructor()
        .property("shapeIdA", &b2ContactData::shapeIdA)
        .property("shapeIdB", &b2ContactData::shapeIdB)
        .property("manifold", &b2ContactData::manifold, return_value_policy::reference())
        ;

    class_<b2ContactEndTouchEvent>("b2ContactEndTouchEvent")
        .constructor()
        .property("shapeIdA", &b2ContactEndTouchEvent::shapeIdA)
        .property("shapeIdB", &b2ContactEndTouchEvent::shapeIdB)
        ;

    class_<b2ContactHitEvent>("b2ContactHitEvent")
        .constructor()
        .property("shapeIdA", &b2ContactHitEvent::shapeIdA)
        .property("shapeIdB", &b2ContactHitEvent::shapeIdB)
        .property("point", &b2ContactHitEvent::point, return_value_policy::reference())
        .property("normal", &b2ContactHitEvent::normal, return_value_policy::reference())
        .property("approachSpeed", &b2ContactHitEvent::approachSpeed)
        ;

    class_<b2Profile>("b2Profile")
        .constructor()
        .property("step", &b2Profile::step)
        .property("pairs", &b2Profile::pairs)
        .property("collide", &b2Profile::collide)
        .property("solve", &b2Profile::solve)
        .property("buildIslands", &b2Profile::buildIslands)
        .property("solveConstraints", &b2Profile::solveConstraints)
        .property("prepareTasks", &b2Profile::prepareTasks)
        .property("solverTasks", &b2Profile::solverTasks)
        .property("prepareConstraints", &b2Profile::prepareConstraints)
        .property("integrateVelocities", &b2Profile::integrateVelocities)
        .property("warmStart", &b2Profile::warmStart)
        .property("solveVelocities", &b2Profile::solveVelocities)
        .property("integratePositions", &b2Profile::integratePositions)
        .property("relaxVelocities", &b2Profile::relaxVelocities)
        .property("applyRestitution", &b2Profile::applyRestitution)
        .property("storeImpulses", &b2Profile::storeImpulses)
        .property("finalizeBodies", &b2Profile::finalizeBodies)
        .property("splitIslands", &b2Profile::splitIslands)
        .property("sleepIslands", &b2Profile::sleepIslands)
        .property("hitEvents", &b2Profile::hitEvents)
        .property("broadphase", &b2Profile::broadphase)
        .property("continuous", &b2Profile::continuous)
        ;


    class_<b2TreeStats>("b2TreeStats")
        .constructor()
        .property("nodeVisits", &b2TreeStats::nodeVisits)
        .property("leafVisits", &b2TreeStats::leafVisits)
        ;

    function("b2DefaultQueryFilter", &b2DefaultQueryFilter);

    class_<b2QueryFilter>("b2QueryFilter")
        .constructor()
        .property("categoryBits",
            +[](const b2QueryFilter& filter) { return static_cast<uint32_t>(filter.categoryBits); },
            +[](b2QueryFilter& filter, uint32_t val) { filter.categoryBits = val; })
        .property("maskBits",
            +[](const b2QueryFilter& filter) { return static_cast<uint32_t>(filter.maskBits); },
            +[](b2QueryFilter& filter, uint32_t val) { filter.maskBits = val; })

        // 64-bit access via string conversion
        .function("setCategoryBits64",
            +[](b2QueryFilter& filter, const std::string& val) {
                filter.categoryBits = std::stoull(val);
            })
        .function("getCategoryBits64",
            +[](const b2QueryFilter& filter) -> std::string {
                return std::to_string(filter.categoryBits);
            })
        .function("setMaskBits64",
            +[](b2QueryFilter& filter, const std::string& val) {
                filter.maskBits = std::stoull(val);
            })
        .function("getMaskBits64",
            +[](const b2QueryFilter& filter) -> std::string {
                return std::to_string(filter.maskBits);
            })
        ;

    function("b2DefaultExplosionDef", &b2DefaultExplosionDef);

    class_<b2ExplosionDef>("b2ExplosionDef")
        .constructor()
        .property("maskBits",
            +[](const b2ExplosionDef& filter) { return static_cast<uint32_t>(filter.maskBits); },
            +[](b2ExplosionDef& filter, uint32_t val) { filter.maskBits = val; })
        .property("position", &b2ExplosionDef::position, return_value_policy::reference())
        .property("radius", &b2ExplosionDef::radius)
        .property("falloff", &b2ExplosionDef::falloff)
        .property("impulsePerLength", &b2ExplosionDef::impulsePerLength)
        ;

    class_<b2RayResult>("b2RayResult")
        .constructor()
        .property("shapeId", &b2RayResult::shapeId)
        .property("point", &b2RayResult::point, return_value_policy::reference())
        .property("normal", &b2RayResult::normal, return_value_policy::reference())
        .property("fraction", &b2RayResult::fraction)
        .property("nodeVisits", &b2RayResult::nodeVisits)
        .property("leafVisits", &b2RayResult::leafVisits)
        .property("hit", &b2RayResult::hit)
        ;

    class_<BasicWorldInterface<World, false>>("BasicWorldInterface");
    class_<MaybeConstWorldRef<false>>("WorldRef");
    class_<MaybeConstWorldRef<true>>("WorldConstRef");
    class_<World, base<BasicWorldInterface<World, false>>>("World")
        .constructor()
        .constructor<const b2WorldDef&>()
        .function("Destroy", &b2::World::Destroy)
        .function("IsValid", &b2::World::IsValid)
        .function("Step", &b2::World::Step)
        .function("SetGravity", &b2::World::SetGravity)
        .function("GetGravity", &b2::World::GetGravity)
        .function("GetAwakeBodyCount", &b2::World::GetAwakeBodyCount)
        .function("GetBodyEvents", &b2::World::GetBodyEvents)
        .function("GetContactEvents", &b2::World::GetContactEvents)
        .function("SetContactTuning", &b2::World::SetContactTuning)
        .function("EnableContinuous", &b2::World::EnableContinuous)
        .function("IsContinuousEnabled", &b2::World::IsContinuousEnabled)
        .function("EnableSleeping", &b2::World::EnableSleeping)
        .function("IsSleepingEnabled", &b2::World::IsSleepingEnabled)
        .function("EnableWarmStarting", &b2::World::EnableWarmStarting)
        .function("IsWarmStartingEnabled", &b2::World::IsWarmStartingEnabled)
        .function("SetMaximumLinearSpeed", &b2::World::SetMaximumLinearSpeed)
        .function("GetMaximumLinearSpeed", &b2::World::GetMaximumLinearSpeed)
        .function("SetRestitutionThreshold", &b2::World::SetRestitutionThreshold)
        .function("GetRestitutionThreshold", &b2::World::GetRestitutionThreshold)
        .function("SetHitEventThreshold", &b2::World::SetHitEventThreshold)
        .function("GetHitEventThreshold", &b2::World::GetHitEventThreshold)
        .function("GetCounters", &b2::World::GetCounters)
        .function("GetProfile", &b2::World::GetProfile)
        .function("GetSensorEvents", &b2::World::GetSensorEvents)
        .function("GetPointer", +[](const World& self) -> emscripten::val {
            return emscripten::val(self.Handle().index1);
        })
        .function("CreateBody", +[](b2::World& world, const b2BodyDef& def) -> Body* {
            Body* body = new Body();
            *body = world.CreateBody(b2::Tags::OwningHandle{}, def);
            return body;
        }, allow_raw_pointers())
        .function("Draw", &b2::World::Draw)
        ;

    // ------------------------------------------------------------------------
    // b2Shape
    // ------------------------------------------------------------------------

     class_<b2Circle>("b2Circle")
        .constructor()
        .property("center", &b2Circle::center, return_value_policy::reference())
        .property("radius", &b2Circle::radius)
        ;

    class_<b2Capsule>("b2Capsule")
        .constructor()
        .property("center1", &b2Capsule::center1, return_value_policy::reference())
        .property("center2", &b2Capsule::center2, return_value_policy::reference())
        .property("radius", &b2Capsule::radius)
        ;

    class_<b2Segment>("b2Segment")
        .constructor()
        .property("point1", &b2Segment::point1, return_value_policy::reference())
        .property("point2", &b2Segment::point2, return_value_policy::reference())
        ;

    class_<b2Filter>("b2Filter")
        .constructor()
        .property("categoryBits",
            +[](const b2Filter& filter) { return static_cast<uint32_t>(filter.categoryBits); },
            +[](b2Filter& filter, uint32_t val) { filter.categoryBits = val; })
        .property("maskBits",
            +[](const b2Filter& filter) { return static_cast<uint32_t>(filter.maskBits); },
            +[](b2Filter& filter, uint32_t val) { filter.maskBits = val; })
        .property("groupIndex", &b2Filter::groupIndex);

    class_<b2ShapeDef>("b2ShapeDef")
        .constructor()
        .property("friction", &b2ShapeDef::friction)
        .property("restitution", &b2ShapeDef::restitution)
        .property("density", &b2ShapeDef::density)
        .property("filter", &b2ShapeDef::filter, return_value_policy::reference())
        .property("customColor", &b2ShapeDef::customColor)
        .property("isSensor", &b2ShapeDef::isSensor)
        .property("enableSensorEvents", &b2ShapeDef::enableSensorEvents)
        .property("enableContactEvents", &b2ShapeDef::enableContactEvents)
        .property("enableHitEvents", &b2ShapeDef::enableHitEvents)
        .property("enablePreSolveEvents", &b2ShapeDef::enablePreSolveEvents)
        .property("invokeContactCreation", &b2ShapeDef::invokeContactCreation)
        .property("updateBodyMass", &b2ShapeDef::updateBodyMass)
        ;

    class_<b2Polygon>("b2Polygon")
        .constructor()
        .function("GetVertex", +[](const b2Polygon& self, int index) -> b2Vec2 {
            if (index < 0 || index >= B2_MAX_POLYGON_VERTICES) return b2Vec2();
            return self.vertices[index];
        })
        .function("SetVertex", +[](b2Polygon& self, int index, const b2Vec2& value) {
            if (index >= 0 && index < B2_MAX_POLYGON_VERTICES) {
                self.vertices[index] = value;
            }
        })
        .function("GetNormal", +[](const b2Polygon& self, int index) -> b2Vec2 {
            if (index < 0 || index >= B2_MAX_POLYGON_VERTICES) return b2Vec2();
            return self.normals[index];
        })
        .function("SetNormal", +[](b2Polygon& self, int index, const b2Vec2& value) {
            if (index >= 0 && index < B2_MAX_POLYGON_VERTICES) {
                self.normals[index] = value;
            }
        })
        .property("centroid", &b2Polygon::centroid, return_value_policy::reference())
        .property("radius", &b2Polygon::radius)
        .property("count", &b2Polygon::count)
        .class_function("GetMaxVertices", +[]() { return B2_MAX_POLYGON_VERTICES; })
    ;

    function("b2DefaultChainDef", &b2DefaultChainDef);

    class_<b2ChainDef>("b2ChainDef")
        .constructor<>()
        .property("friction", &b2ChainDef::friction)
        .property("restitution", &b2ChainDef::restitution)
        .property("filter", &b2ChainDef::filter, return_value_policy::reference())
        .property("customColor", &b2ChainDef::customColor)
        .property("isLoop", &b2ChainDef::isLoop)
        .function("GetPoints", +[](const b2ChainDef& self) -> emscripten::val {
            auto result = emscripten::val::array();
            for (int i = 0; i < self.count; i++) {
                auto point = emscripten::val::object();
                point.set("x", self.points[i].x);
                point.set("y", self.points[i].y);
                result.set(i, point);
            }
            return result;
        })
        .function("SetPoints", +[](b2ChainDef& self, const emscripten::val& points) {
            int length = points["length"].as<int>();
            b2Vec2* newPoints = new b2Vec2[length];

            for (int i = 0; i < length; i++) {
                auto point = points[i];
                newPoints[i].x = point["x"].as<float>();
                newPoints[i].y = point["y"].as<float>();
            }
            if (self.points != nullptr) {
                delete[] self.points;
            }

            self.points = newPoints;
            self.count = length;
        })
        .property("count", &b2ChainDef::count)
        .property("internalValue", &b2ChainDef::internalValue);

    class_<BasicChainInterface<Chain, false>>("BasicChainInterface");

    class_<Chain, base<BasicChainInterface<Chain, false>>>("Chain")
        .constructor<>()
        .function("Destroy", &Chain::Destroy)
        .function("IsValid", &Chain::IsValid)
        .function("SetFriction", &Chain::SetFriction)
        .function("GetFriction", &Chain::GetFriction)
        .function("SetRestitution", &Chain::SetRestitution)
        .function("GetRestitution", &Chain::GetRestitution)
        .function("GetSegmentCount", &Chain::GetSegmentCount)
        .function("GetSegments", +[](const Chain& self) {
            int count = self.GetSegmentCount();
            if (count == 0) return emscripten::val::array();

            std::vector<b2ShapeId> segments(count);
            int actual = self.GetSegments(segments.data(), count);

            auto result = emscripten::val::array();
            for (int i = 0; i < actual; i++) {
                result.set(i, segments[i]);
            }
            return result;
        })
        .function("GetWorld", select_overload<WorldRef()>(&Chain::GetWorld))
        .function("GetPointer", +[](const Chain& self) -> emscripten::val {
            return emscripten::val(self.Handle().index1);
        })
        ;

    class_<b2ChainId>("b2ChainId")
        .constructor()
        .property("index1", &b2ChainId::index1)
        .property("world0", &b2ChainId::world0)
        .property("revision", &b2ChainId::revision)
        ;

    class_<b2ChainSegment>("b2ChainSegment")
        .constructor()
        .property("ghost1", &b2ChainSegment::ghost1, return_value_policy::reference())
        .property("segment", &b2ChainSegment::segment, return_value_policy::reference())
        .property("ghost2", &b2ChainSegment::ghost2, return_value_policy::reference())
        .property("chainId", &b2ChainSegment::chainId)
        ;

    function("b2MakePolygon", &b2MakePolygon, allow_raw_pointers());
    function("b2MakeOffsetPolygon", &b2MakeOffsetPolygon, allow_raw_pointers());
    function("b2MakeOffsetRoundedPolygon", &b2MakeOffsetRoundedPolygon, allow_raw_pointers());
    function("b2MakeSquare", &b2MakeSquare);
    function("b2MakeBox", &b2MakeBox);
    function("b2MakeRoundedBox", &b2MakeRoundedBox);
    function("b2MakeOffsetBox", &b2MakeOffsetBox);
    function("b2MakeOffsetRoundedBox", &b2MakeOffsetRoundedBox);
    function("b2TransformPolygon", &b2TransformPolygon, allow_raw_pointers());
    function("b2ComputeCircleMass", &b2ComputeCircleMass, allow_raw_pointers());
    function("b2ComputeCapsuleMass", &b2ComputeCapsuleMass, allow_raw_pointers());
    function("b2ComputePolygonMass", &b2ComputePolygonMass, allow_raw_pointers());
    function("b2ComputeCircleAABB", &b2ComputeCircleAABB, allow_raw_pointers());
    function("b2ComputeCapsuleAABB", &b2ComputeCapsuleAABB, allow_raw_pointers());
    function("b2ComputePolygonAABB", &b2ComputePolygonAABB, allow_raw_pointers());
    function("b2ComputeSegmentAABB", &b2ComputeSegmentAABB, allow_raw_pointers());
    function("b2PointInCircle", &b2PointInCircle, allow_raw_pointers());
    function("b2PointInCapsule", &b2PointInCapsule, allow_raw_pointers());
    function("b2PointInPolygon", &b2PointInPolygon, allow_raw_pointers());
    function("b2IsValidRay", &b2IsValidRay, allow_raw_pointers());
    // function("b2RayCastCircle", &b2RayCastCircle, allow_raw_pointers());
    // function("b2RayCastCapsule", &b2RayCastCapsule, allow_raw_pointers());
    // function("b2RayCastSegment", &b2RayCastSegment, allow_raw_pointers());
    // function("b2RayCastPolygon", &b2RayCastPolygon, allow_raw_pointers());
    // function("b2ShapeCastCircle", &b2ShapeCastCircle, allow_raw_pointers());
    // function("b2ShapeCastCapsule", &b2ShapeCastCapsule, allow_raw_pointers());
    // function("b2ShapeCastSegment", &b2ShapeCastSegment, allow_raw_pointers());
    // function("b2ShapeCastPolygon", &b2ShapeCastPolygon, allow_raw_pointers());

    enum_<b2ShapeType>("b2ShapeType")
        .value("b2_circleShape", b2_circleShape)
        .value("b2_capsuleShape", b2_capsuleShape)
        .value("b2_segmentShape", b2_segmentShape)
        .value("b2_polygonShape", b2_polygonShape)
        .value("b2_chainSegmentShape", b2_chainSegmentShape)
        .value("b2_shapeTypeCount", b2_shapeTypeCount)
        ;

    class_<BasicShapeInterface<Shape, false>>("BasicShapeInterface");

    class_<Shape, base<BasicShapeInterface<Shape, false>>>("Shape")
        .constructor()
        .function("Destroy", &Shape::Destroy)
        .function("IsValid", &Shape::IsValid)
        .function("GetAABB", &Shape::GetAABB)
        .function("GetDensity", &Shape::GetDensity)
        .function("SetDensity", &Shape::SetDensity)
        .function("GetFilter", &Shape::GetFilter)
        .function("SetFilter", &Shape::SetFilter)
        .function("GetFriction", &Shape::GetFriction)
        .function("SetFriction", &Shape::SetFriction)
        .function("GetContactData", +[](const Shape& shape) {
            return getArrayWrapper<b2ContactData>(shape, &Shape::GetContactCapacity, &Shape::GetContactData);
        })
        .function("GetContactCapacity", &Shape::GetContactCapacity)
        .function("EnableContactEvents", &Shape::EnableContactEvents)
        .function("AreContactEventsEnabled", &Shape::AreContactEventsEnabled)
        .function("EnableHitEvents", &Shape::EnableHitEvents)
        .function("AreHitEventsEnabled", &Shape::AreHitEventsEnabled)
        .function("IsSensor", &Shape::IsSensor)
        .function("GetSensorCapacity", &Shape::GetSensorCapacity)
        .function("GetSensorOverlaps", +[](const Shape& shape) {
            return getArrayWrapper<b2ShapeId>(shape, &Shape::GetSensorCapacity, &Shape::GetSensorOverlaps);
        })
        .function("EnableSensorEvents", &Shape::EnableSensorEvents)
        .function("AreSensorEventsEnabled", &Shape::AreSensorEventsEnabled)
        .function("EnablePreSolveEvents", &Shape::EnablePreSolveEvents)
        .function("ArePreSolveEventsEnabled", &Shape::ArePreSolveEventsEnabled)
        .function("GetClosestPoint", &Shape::GetClosestPoint)
        .function("RayCast", &Shape::RayCast)
        .function("TestPoint", &Shape::TestPoint)
        .function("GetType", &Shape::GetType)
        .function("GetRestitution", &Shape::GetRestitution)
        .function("SetRestitution", &Shape::SetRestitution)
        .function("GetBody", select_overload<BodyRef()>(&Shape::GetBody))
        .function("GetParentChain", select_overload<ChainRef()>(&Shape::GetParentChain))
        .function("GetWorld", select_overload<WorldRef()>(&Shape::GetWorld))
        .function("GetPointer", +[](const Shape& self) -> emscripten::val {
            return emscripten::val(self.Handle().index1);
        })
        ;

    function("b2DefaultShapeDef", &b2DefaultShapeDef);
    function("b2CreateChain", &b2CreateChain, allow_raw_pointers());
    function("b2CreatePolygonShape", &b2CreatePolygonShape, allow_raw_pointers());
    function("b2CreateCircleShape", &b2CreateCircleShape, allow_raw_pointers());
    function("b2CreateCapsuleShape", &b2CreateCapsuleShape, allow_raw_pointers());
    function("b2CreateSegmentShape", &b2CreateSegmentShape, allow_raw_pointers());
    function("b2DestroyShape", &b2DestroyShape);

    // ------------------------------------------------------------------------
    // b2Body
    // ------------------------------------------------------------------------

    class_<b2MassData>("b2MassData")
        .constructor()
        .property("mass", &b2MassData::mass)
        .property("center", &b2MassData::center, return_value_policy::reference())
        .property("rotationalInertia", &b2MassData::rotationalInertia);
        ;

    enum_<b2BodyType>("b2BodyType")
        .value("b2_staticBody", b2_staticBody)
        .value("b2_kinematicBody", b2_kinematicBody)
        .value("b2_dynamicBody", b2_dynamicBody)
        .value("b2_bodyTypeCount", b2_bodyTypeCount)
        ;

    class_<b2BodyDef>("b2BodyDef")
        .constructor()
        .constructor<const b2BodyDef&>()
        .property("type", &b2BodyDef::type)
        .property("position", &b2BodyDef::position, return_value_policy::reference())
        .property("rotation", &b2BodyDef::rotation, return_value_policy::reference())
        .property("linearVelocity", &b2BodyDef::linearVelocity, return_value_policy::reference())
        .property("angularVelocity", &b2BodyDef::angularVelocity)
        .property("linearDamping", &b2BodyDef::linearDamping)
        .property("angularDamping", &b2BodyDef::angularDamping)
        .property("gravityScale", &b2BodyDef::gravityScale)
        .property("sleepThreshold", &b2BodyDef::sleepThreshold)
        .property("enableSleep", &b2BodyDef::enableSleep)
        .property("isAwake", &b2BodyDef::isAwake)
        .property("fixedRotation", &b2BodyDef::fixedRotation)
        .property("isBullet", &b2BodyDef::isBullet)
        .property("isEnabled", &b2BodyDef::isEnabled)
        .property("allowFastRotation", &b2BodyDef::allowFastRotation)
        .property("internalValue", &b2BodyDef::internalValue)
        ;

    class_<BasicBodyInterface<Body, false>>("BasicBodyInterface");

    class_<Body, base<BasicBodyInterface<Body, false>>>("Body")
        .constructor<>()
        .function("IsValid", &Body::IsValid)
        .function("CreateChain", +[](Body& body, const b2ChainDef& def) -> Chain* {
            Chain* chain = new Chain();
            *chain = body.CreateChain(Tags::OwningHandle{}, def);
            return chain;
        }, emscripten::allow_raw_pointers())
        .function("CreateCapsuleShape", +[](Body& body, const b2ShapeDef& def, const b2Capsule& capsule) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, capsule);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateCircleShape", +[](Body& body, const b2ShapeDef& def, const b2Circle& circle) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, circle);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreatePolygonShape", +[](Body& body, const b2ShapeDef& def, const b2Polygon& polygon) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, polygon);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("CreateSegmentShape", +[](Body& body, const b2ShapeDef& def, const b2Segment& segment) -> Shape* {
            Shape* shape = new Shape();
            *shape = body.CreateShape(Tags::OwningHandle{}, def, segment);
            return shape;
        }, emscripten::allow_raw_pointers())
        .function("Destroy", &Body::Destroy)
        .function("Enable", &Body::Enable)
        .function("Disable", &Body::Disable)
        .function("IsEnabled", &Body::IsEnabled)
        .function("SetAngularDamping", &Body::SetAngularDamping)
        .function("GetAngularDamping", &Body::GetAngularDamping)
        .function("SetAngularVelocity", &Body::SetAngularVelocity)
        .function("GetAngularVelocity", &Body::GetAngularVelocity)
        .function("ApplyAngularImpulse", &Body::ApplyAngularImpulse)
        .function("ApplyForce", &Body::ApplyForce)
        .function("ApplyForceToCenter", &Body::ApplyForceToCenter)
        .function("ApplyLinearImpulse", &Body::ApplyLinearImpulse)
        .function("ApplyLinearImpulseToCenter", &Body::ApplyLinearImpulseToCenter)
        .function("ApplyTorque", &Body::ApplyTorque)
        .function("ApplyMassFromShapes", &Body::ApplyMassFromShapes)
        .function("GetMass", &Body::GetMass)
        .function("SetMassData", &Body::SetMassData)
        .function("GetMassData", &Body::GetMassData)
        .function("GetRotationalInertia", &Body::GetRotationalInertia)
        .function("SetAwake", &Body::SetAwake)
        .function("IsAwake", &Body::IsAwake)
        .function("EnableSleep", &Body::EnableSleep)
        .function("IsSleepEnabled", &Body::IsSleepEnabled)
        .function("SetSleepThreshold", &Body::SetSleepThreshold)
        .function("GetSleepThreshold", &Body::GetSleepThreshold)
        .function("SetBullet", &Body::SetBullet)
        .function("IsBullet", &Body::IsBullet)
        .function("ComputeAABB", &Body::ComputeAABB)
        .function("GetContactCapacity", &Body::GetContactCapacity)
        .function("GetContactData", +[](const Body& body) {
            return getArrayWrapper<b2ContactData>(body, &Body::GetContactCapacity, &Body::GetContactData);
        })
        .function("EnableContactEvents", &Body::EnableContactEvents)
        .function("SetFixedRotation", &Body::SetFixedRotation)
        .function("IsFixedRotation", &Body::IsFixedRotation)
        .function("SetGravityScale", &Body::SetGravityScale)
        .function("GetGravityScale", &Body::GetGravityScale)
        .function("EnableHitEvents", &Body::EnableHitEvents)
        .function("GetJointCount", &Body::GetJointCount)
        .function("GetJoints", +[](const Body& body) {
            return getArrayWrapper<b2JointId>(body, &Body::GetJointCount, &Body::GetJoints);
        })
        .function("SetLinearDamping", &Body::SetLinearDamping)
        .function("GetLinearDamping", &Body::GetLinearDamping)
        .function("SetLinearVelocity", &Body::SetLinearVelocity)
        .function("GetLinearVelocity", &Body::GetLinearVelocity)
        .function("GetLocalCenterOfMass", &Body::GetLocalCenterOfMass)
        .function("GetLocalPoint", &Body::GetLocalPoint)
        .function("GetLocalVector", &Body::GetLocalVector)
        .function("GetPosition", &Body::GetPosition)
        .function("GetRotation", &Body::GetRotation)
        .function("EnableSensorEvents", &Body::EnableSensorEvents)
        .function("SetTransform", &Body::SetTransform)
        .function("GetTransform", &Body::GetTransform)
        .function("GetShapeCount", &Body::GetShapeCount)
        .function("GetShapes", +[](const Body& body) {
            return getArrayWrapper<b2ShapeId>(body, &Body::GetShapeCount, &Body::GetShapes);
        })
        .function("SetType", &Body::SetType)
        .function("GetType", &Body::GetType)
        .function("GetPointer", +[](const Body& self) -> emscripten::val {
            return emscripten::val(self.Handle().index1);
        })
        .function("GetWorld", select_overload<WorldRef()>(&Body::GetWorld))
        .function("GetWorldCenterOfMass", &Body::GetWorldCenterOfMass)
        .function("GetWorldPoint", &Body::GetWorldPoint)
        .function("GetWorldVector", &Body::GetWorldVector)
        ;

    // ------------------------------------------------------------------------
    // b2Joint
    // ------------------------------------------------------------------------
    enum_<b2JointType>("b2JointType")
        .value("b2_distanceJoint", b2JointType::b2_distanceJoint)
        .value("b2_motorJoint", b2JointType::b2_motorJoint)
        .value("b2_mouseJoint", b2JointType::b2_mouseJoint)
        .value("b2_nullJoint", b2JointType::b2_nullJoint)
        .value("b2_prismaticJoint", b2JointType::b2_prismaticJoint)
        .value("b2_revoluteJoint", b2JointType::b2_revoluteJoint)
        .value("b2_weldJoint", b2JointType::b2_weldJoint)
        .value("b2_wheelJoint", b2JointType::b2_wheelJoint)
        ;

    class_<b2::MaybeConstBodyRef<false>>("BodyRef");
    class_<b2::MaybeConstBodyRef<true>>("BodyConstRef");
    class_<b2::MaybeConstChainRef<false>>("ChainRef");
    class_<b2::MaybeConstChainRef<true>>("ChainConstRef");
    class_<BasicJointInterface<Joint, false>>("BasicJointInterface");

    class_<Joint, base<BasicJointInterface<Joint, false>>>("Joint")
        .constructor<>()
        .function("Destroy", &Joint::Destroy)
        .function("IsValid", &Joint::IsValid)
        .function("GetBodyA", select_overload<BodyRef()>(&Joint::GetBodyA))
        .function("GetBodyB", select_overload<BodyRef()>(&Joint::GetBodyB))
        .function("SetCollideConnected", &Joint::SetCollideConnected)
        .function("GetCollideConnected", &Joint::GetCollideConnected)
        .function("GetConstraintForce", &Joint::GetConstraintForce)
        .function("GetConstraintTorque", &Joint::GetConstraintTorque)
        .function("GetLocalAnchorA", &Joint::GetLocalAnchorA, return_value_policy::reference())
        .function("GetLocalAnchorB", &Joint::GetLocalAnchorB, return_value_policy::reference())
        .function("GetType", &Joint::GetType)
        .function("WakeBodies", &Joint::WakeBodies)
        .function("GetWorld", select_overload<WorldRef()>(&Joint::GetWorld))
        .function("GetPointer", +[](const Joint& self) -> emscripten::val {
            return emscripten::val(self.Handle().index1);
        })
        ;

    class_<b2::MaybeConstJointRef<false>>("JointRef");
    class_<b2::MaybeConstJointRef<true>>("JointConstRef");
    class_<b2::MaybeConstDistanceJointRef<false>>("DistanceJointRef");
    class_<b2::MaybeConstDistanceJointRef<true>>("DistanceJointConstRef");
    class_<b2::MaybeConstMotorJointRef<false>>("MotorJointRef");
    class_<b2::MaybeConstMotorJointRef<true>>("MotorJointConstRef");
    class_<b2::MaybeConstMouseJointRef<false>>("MouseJointRef");
    class_<b2::MaybeConstMouseJointRef<true>>("MouseJointConstRef");
    class_<b2::MaybeConstPrismaticJointRef<false>>("PrismaticJointRef");
    class_<b2::MaybeConstPrismaticJointRef<true>>("PrismaticJointConstRef");
    class_<b2::MaybeConstRevoluteJointRef<false>>("RevoluteJointRef");
    class_<b2::MaybeConstRevoluteJointRef<true>>("RevoluteJointConstRef");
    class_<b2::MaybeConstWeldJointRef<false>>("WeldJointRef");
    class_<b2::MaybeConstWeldJointRef<true>>("WeldJointConstRef");
    class_<b2::MaybeConstWheelJointRef<false>>("WheelJointRef");
    class_<b2::MaybeConstWheelJointRef<true>>("WheelJointConstRef");

    class_<BasicDistanceJointInterface<DistanceJoint, false>>("BasicDistanceJointInterface");
    class_<BasicMotorJointInterface<MotorJoint, false>>("BasicMotorJointInterface");
    class_<BasicMouseJointInterface<MouseJoint, false>>("BasicMouseJointInterface");
    class_<BasicPrismaticJointInterface<PrismaticJoint, false>>("BasicPrismaticJointInterface");
    class_<BasicRevoluteJointInterface<RevoluteJoint, false>>("BasicRevoluteJointInterface");
    class_<BasicWeldJointInterface<WeldJoint, false>>("BasicWeldJointInterface");
    class_<BasicWheelJointInterface<WheelJoint, false>>("BasicWheelJointInterface");

    class_<b2JointId>("b2JointId")
        .constructor()
        .property("index1", &b2JointId::index1)
        .property("world0", &b2JointId::world0)
        .property("revision", &b2JointId::revision)
        ;


    class_<b2DistanceJointDef>("b2DistanceJointDef")
        .constructor()
        .constructor<const b2DistanceJointDef&>()
        .property("bodyIdA", &b2DistanceJointDef::bodyIdA)
        .property("bodyIdB", &b2DistanceJointDef::bodyIdB)
        .property("localAnchorA", &b2DistanceJointDef::localAnchorA, return_value_policy::reference())
        .property("localAnchorB", &b2DistanceJointDef::localAnchorB, return_value_policy::reference())
        .property("length", &b2DistanceJointDef::length)
        .property("enableSpring", &b2DistanceJointDef::enableSpring)
        .property("hertz", &b2DistanceJointDef::hertz)
        .property("dampingRatio", &b2DistanceJointDef::dampingRatio)
        .property("enableLimit", &b2DistanceJointDef::enableLimit)
        .property("minLength", &b2DistanceJointDef::minLength)
        .property("maxLength", &b2DistanceJointDef::maxLength)
        .property("enableMotor", &b2DistanceJointDef::enableMotor)
        .property("maxMotorForce", &b2DistanceJointDef::maxMotorForce)
        .property("motorSpeed", &b2DistanceJointDef::motorSpeed)
        .property("collideConnected", &b2DistanceJointDef::collideConnected)
    ;

    class_<b2MotorJointDef>("b2MotorJointDef")
        .constructor()
        .constructor<const b2MotorJointDef&>()
        .property("bodyIdA", &b2MotorJointDef::bodyIdA)
        .property("bodyIdB", &b2MotorJointDef::bodyIdB)
        .property("linearOffset", &b2MotorJointDef::linearOffset)
        .property("angularOffset", &b2MotorJointDef::angularOffset)
        .property("maxForce", &b2MotorJointDef::maxForce)
        .property("maxTorque", &b2MotorJointDef::maxTorque)
        .property("correctionFactor", &b2MotorJointDef::correctionFactor)
        .property("collideConnected", &b2MotorJointDef::collideConnected)
    ;

    class_<b2MouseJointDef>("b2MouseJointDef")
        .constructor()
        .constructor<const b2MouseJointDef&>()
        .property("bodyIdA", &b2MouseJointDef::bodyIdA)
        .property("bodyIdB", &b2MouseJointDef::bodyIdB)
        .property("target", &b2MouseJointDef::target)
        .property("hertz", &b2MouseJointDef::hertz)
        .property("dampingRatio", &b2MouseJointDef::dampingRatio)
        .property("maxForce", &b2MouseJointDef::maxForce)
        .property("collideConnected", &b2MouseJointDef::collideConnected)
    ;

    class_<b2NullJointDef>("b2NullJointDef")
        .constructor()
        .constructor<const b2NullJointDef&>()
        .property("bodyIdA", &b2NullJointDef::bodyIdA)
        .property("bodyIdB", &b2NullJointDef::bodyIdB)
    ;

    class_<b2PrismaticJointDef>("b2PrismaticJointDef")
        .constructor()
        .constructor<const b2PrismaticJointDef&>()
        .property("bodyIdA", &b2PrismaticJointDef::bodyIdA)
        .property("bodyIdB", &b2PrismaticJointDef::bodyIdB)
        .property("localAnchorA", &b2PrismaticJointDef::localAnchorA, return_value_policy::reference())
        .property("localAnchorB", &b2PrismaticJointDef::localAnchorB, return_value_policy::reference())
        .property("localAxisA", &b2PrismaticJointDef::localAxisA)
        .property("referenceAngle", &b2PrismaticJointDef::referenceAngle)
        .property("enableSpring", &b2PrismaticJointDef::enableSpring)
        .property("hertz", &b2PrismaticJointDef::hertz)
        .property("dampingRatio", &b2PrismaticJointDef::dampingRatio)
        .property("enableLimit", &b2PrismaticJointDef::enableLimit)
        .property("lowerTranslation", &b2PrismaticJointDef::lowerTranslation)
        .property("upperTranslation", &b2PrismaticJointDef::upperTranslation)
        .property("enableMotor", &b2PrismaticJointDef::enableMotor)
        .property("maxMotorForce", &b2PrismaticJointDef::maxMotorForce)
        .property("motorSpeed", &b2PrismaticJointDef::motorSpeed)
        .property("collideConnected", &b2PrismaticJointDef::collideConnected)
    ;

    class_<b2RevoluteJointDef>("b2RevoluteJointDef")
        .constructor()
        .constructor<const b2RevoluteJointDef&>()
        .property("bodyIdA", &b2RevoluteJointDef::bodyIdA)
        .property("bodyIdB", &b2RevoluteJointDef::bodyIdB)
        .property("localAnchorA", &b2RevoluteJointDef::localAnchorA, return_value_policy::reference())
        .property("localAnchorB", &b2RevoluteJointDef::localAnchorB, return_value_policy::reference())
        .property("referenceAngle", &b2RevoluteJointDef::referenceAngle)
        .property("enableSpring", &b2RevoluteJointDef::enableSpring)
        .property("hertz", &b2RevoluteJointDef::hertz)
        .property("dampingRatio", &b2RevoluteJointDef::dampingRatio)
        .property("enableLimit", &b2RevoluteJointDef::enableLimit)
        .property("lowerAngle", &b2RevoluteJointDef::lowerAngle)
        .property("upperAngle", &b2RevoluteJointDef::upperAngle)
        .property("enableMotor", &b2RevoluteJointDef::enableMotor)
        .property("maxMotorTorque", &b2RevoluteJointDef::maxMotorTorque)
        .property("motorSpeed", &b2RevoluteJointDef::motorSpeed)
        .property("drawSize", &b2RevoluteJointDef::drawSize)
        .property("collideConnected", &b2RevoluteJointDef::collideConnected)
    ;

    class_<b2WeldJointDef>("b2WeldJointDef")
        .constructor()
        .constructor<const b2WeldJointDef&>()
        .property("bodyIdA", &b2WeldJointDef::bodyIdA)
        .property("bodyIdB", &b2WeldJointDef::bodyIdB)
        .property("localAnchorA", &b2WeldJointDef::localAnchorA, return_value_policy::reference())
        .property("localAnchorB", &b2WeldJointDef::localAnchorB, return_value_policy::reference())
        .property("referenceAngle", &b2WeldJointDef::referenceAngle)
        .property("linearHertz", &b2WeldJointDef::linearHertz)
        .property("angularHertz", &b2WeldJointDef::angularHertz)
        .property("linearDampingRatio", &b2WeldJointDef::linearDampingRatio)
        .property("angularDampingRatio", &b2WeldJointDef::angularDampingRatio)
        .property("collideConnected", &b2WeldJointDef::collideConnected)
    ;

    class_<b2WheelJointDef>("b2WheelJointDef")
        .constructor()
        .constructor<const b2WheelJointDef&>()
        .property("bodyIdA", &b2WheelJointDef::bodyIdA)
        .property("bodyIdB", &b2WheelJointDef::bodyIdB)
        .property("localAnchorA", &b2WheelJointDef::localAnchorA, return_value_policy::reference())
        .property("localAnchorB", &b2WheelJointDef::localAnchorB, return_value_policy::reference())
        .property("localAxisA", &b2WheelJointDef::localAxisA, return_value_policy::reference())
        .property("enableSpring", &b2WheelJointDef::enableSpring)
        .property("hertz", &b2WheelJointDef::hertz)
        .property("dampingRatio", &b2WheelJointDef::dampingRatio)
        .property("enableLimit", &b2WheelJointDef::enableLimit)
        .property("lowerTranslation", &b2WheelJointDef::lowerTranslation)
        .property("upperTranslation", &b2WheelJointDef::upperTranslation)
        .property("enableMotor", &b2WheelJointDef::enableMotor)
        .property("maxMotorTorque", &b2WheelJointDef::maxMotorTorque)
        .property("motorSpeed", &b2WheelJointDef::motorSpeed)
        .property("collideConnected", &b2WheelJointDef::collideConnected)
    ;

    class_<DistanceJoint, base<BasicDistanceJointInterface<DistanceJoint, false>>>("DistanceJoint")
        .constructor<>()
        .function("GetCurrentLength", &DistanceJoint::GetCurrentLength)
        .function("SetLength", &DistanceJoint::SetLength)
        .function("GetLength", &DistanceJoint::GetLength)
        .function("SetLengthRange", &DistanceJoint::SetLengthRange)
        .function("EnableLimit", &DistanceJoint::EnableLimit)
        .function("IsLimitEnabled", &DistanceJoint::IsLimitEnabled)
        .function("GetMaxLength", &DistanceJoint::GetMaxLength)
        .function("SetMaxMotorForce", &DistanceJoint::SetMaxMotorForce)
        .function("GetMaxMotorForce", &DistanceJoint::GetMaxMotorForce)
        .function("GetMinLength", &DistanceJoint::GetMinLength)
        .function("EnableMotor", &DistanceJoint::EnableMotor)
        .function("IsMotorEnabled", &DistanceJoint::IsMotorEnabled)
        .function("GetMotorForce", &DistanceJoint::GetMotorForce)
        .function("SetMotorSpeed", &DistanceJoint::SetMotorSpeed)
        .function("GetMotorSpeed", &DistanceJoint::GetMotorSpeed)
        .function("EnableSpring", &DistanceJoint::EnableSpring)
        .function("IsSpringEnabled", &DistanceJoint::IsSpringEnabled)
        .function("SetSpringDampingRatio", &DistanceJoint::SetSpringDampingRatio)
        .function("GetSpringDampingRatio", &DistanceJoint::GetSpringDampingRatio)
        .function("SetSpringHertz", &DistanceJoint::SetSpringHertz)
        .function("GetSpringHertz", &DistanceJoint::GetSpringHertz);

    class_<MotorJoint, base<BasicMotorJointInterface<MotorJoint, false>>>("MotorJoint")
        .constructor<>()
        .function("SetAngularOffset", &MotorJoint::SetAngularOffset)
        .function("GetAngularOffset", &MotorJoint::GetAngularOffset)
        .function("SetCorrectionFactor", &MotorJoint::SetCorrectionFactor)
        .function("GetCorrectionFactor", &MotorJoint::GetCorrectionFactor)
        .function("SetLinearOffset", &MotorJoint::SetLinearOffset)
        .function("GetLinearOffset", &MotorJoint::GetLinearOffset)
        .function("SetMaxForce", &MotorJoint::SetMaxForce)
        .function("GetMaxForce", &MotorJoint::GetMaxForce)
        .function("SetMaxTorque", &MotorJoint::SetMaxTorque)
        .function("GetMaxTorque", &MotorJoint::GetMaxTorque);

    class_<MouseJoint, base<BasicMouseJointInterface<MouseJoint, false>>>("MouseJoint")
        .constructor<>()
        .function("SetMaxForce", &MouseJoint::SetMaxForce)
        .function("GetMaxForce", &MouseJoint::GetMaxForce)
        .function("SetSpringDampingRatio", &MouseJoint::SetSpringDampingRatio)
        .function("GetSpringDampingRatio", &MouseJoint::GetSpringDampingRatio)
        .function("SetSpringHertz", &MouseJoint::SetSpringHertz)
        .function("GetSpringHertz", &MouseJoint::GetSpringHertz)
        .function("SetTarget", &MouseJoint::SetTarget)
        .function("GetTarget", &MouseJoint::GetTarget);

    class_<PrismaticJoint, base<BasicPrismaticJointInterface<PrismaticJoint, false>>>("PrismaticJoint")
        .constructor<>()
        .function("EnableLimit", &PrismaticJoint::EnableLimit)
        .function("IsLimitEnabled", &PrismaticJoint::IsLimitEnabled)
        .function("SetLimits", &PrismaticJoint::SetLimits)
        .function("GetLowerLimit", &PrismaticJoint::GetLowerLimit)
        .function("SetMaxMotorForce", &PrismaticJoint::SetMaxMotorForce)
        .function("GetMaxMotorForce", &PrismaticJoint::GetMaxMotorForce)
        .function("EnableMotor", &PrismaticJoint::EnableMotor)
        .function("IsMotorEnabled", &PrismaticJoint::IsMotorEnabled)
        .function("GetMotorForce", &PrismaticJoint::GetMotorForce)
        .function("SetMotorSpeed", &PrismaticJoint::SetMotorSpeed)
        .function("GetMotorSpeed", &PrismaticJoint::GetMotorSpeed)
        .function("GetSpeed", &PrismaticJoint::GetSpeed)
        .function("EnableSpring", &PrismaticJoint::EnableSpring)
        .function("IsSpringEnabled", &PrismaticJoint::IsSpringEnabled)
        .function("SetSpringDampingRatio", &PrismaticJoint::SetSpringDampingRatio)
        .function("GetSpringDampingRatio", &PrismaticJoint::GetSpringDampingRatio)
        .function("SetSpringHertz", &PrismaticJoint::SetSpringHertz)
        .function("GetSpringHertz", &PrismaticJoint::GetSpringHertz)
        .function("GetTranslation", &PrismaticJoint::GetTranslation)
        .function("GetUpperLimit", &PrismaticJoint::GetUpperLimit);

    class_<RevoluteJoint, base<BasicRevoluteJointInterface<RevoluteJoint, false>>>("RevoluteJoint")
        .constructor<>()
        .function("GetAngle", &RevoluteJoint::GetAngle)
        .function("EnableLimit", &RevoluteJoint::EnableLimit)
        .function("IsLimitEnabled", &RevoluteJoint::IsLimitEnabled)
        .function("SetLimits", &RevoluteJoint::SetLimits)
        .function("GetLowerLimit", &RevoluteJoint::GetLowerLimit)
        .function("SetMaxMotorTorque", &RevoluteJoint::SetMaxMotorTorque)
        .function("GetMaxMotorTorque", &RevoluteJoint::GetMaxMotorTorque)
        .function("EnableMotor", &RevoluteJoint::EnableMotor)
        .function("IsMotorEnabled", &RevoluteJoint::IsMotorEnabled)
        .function("SetMotorSpeed", &RevoluteJoint::SetMotorSpeed)
        .function("GetMotorSpeed", &RevoluteJoint::GetMotorSpeed)
        .function("GetMotorTorque", &RevoluteJoint::GetMotorTorque)
        .function("EnableSpring", &RevoluteJoint::EnableSpring)
        .function("IsSpringEnabled", &RevoluteJoint::IsSpringEnabled)
        .function("SetSpringDampingRatio", &RevoluteJoint::SetSpringDampingRatio)
        .function("GetSpringDampingRatio", &RevoluteJoint::GetSpringDampingRatio)
        .function("SetSpringHertz", &RevoluteJoint::SetSpringHertz)
        .function("GetSpringHertz", &RevoluteJoint::GetSpringHertz)
        .function("GetUpperLimit", &RevoluteJoint::GetUpperLimit);

    class_<WeldJoint, base<BasicWeldJointInterface<WeldJoint, false>>>("WeldJoint")
        .constructor<>()
        .function("SetAngularDampingRatio", &WeldJoint::SetAngularDampingRatio)
        .function("GetAngularDampingRatio", &WeldJoint::GetAngularDampingRatio)
        .function("SetAngularHertz", &WeldJoint::SetAngularHertz)
        .function("GetAngularHertz", &WeldJoint::GetAngularHertz)
        .function("SetLinearDampingRatio", &WeldJoint::SetLinearDampingRatio)
        .function("GetLinearDampingRatio", &WeldJoint::GetLinearDampingRatio)
        .function("SetLinearHertz", &WeldJoint::SetLinearHertz)
        .function("GetLinearHertz", &WeldJoint::GetLinearHertz)
        .function("SetReferenceAngle", &WeldJoint::SetReferenceAngle)
        .function("GetReferenceAngle", &WeldJoint::GetReferenceAngle);

    class_<WheelJoint, base<BasicWheelJointInterface<WheelJoint, false>>>("WheelJoint")
        .constructor<>()
        .function("EnableLimit", &WheelJoint::EnableLimit)
        .function("IsLimitEnabled", &WheelJoint::IsLimitEnabled)
        .function("SetLimits", &WheelJoint::SetLimits)
        .function("GetLowerLimit", &WheelJoint::GetLowerLimit)
        .function("SetMaxMotorTorque", &WheelJoint::SetMaxMotorTorque)
        .function("GetMaxMotorTorque", &WheelJoint::GetMaxMotorTorque)
        .function("EnableMotor", &WheelJoint::EnableMotor)
        .function("IsMotorEnabled", &WheelJoint::IsMotorEnabled)
        .function("SetMotorSpeed", &WheelJoint::SetMotorSpeed)
        .function("GetMotorSpeed", &WheelJoint::GetMotorSpeed)
        .function("GetMotorTorque", &WheelJoint::GetMotorTorque)
        .function("EnableSpring", &WheelJoint::EnableSpring)
        .function("IsSpringEnabled", &WheelJoint::IsSpringEnabled)
        .function("SetSpringDampingRatio", &WheelJoint::SetSpringDampingRatio)
        .function("GetSpringDampingRatio", &WheelJoint::GetSpringDampingRatio)
        .function("SetSpringHertz", &WheelJoint::SetSpringHertz)
        .function("GetSpringHertz", &WheelJoint::GetSpringHertz)
        .function("GetUpperLimit", &WheelJoint::GetUpperLimit);

    // ------------------------------------------------------------------------
    // b2DebugDraw
    // ------------------------------------------------------------------------

    class_<b2DebugDraw>("b2DebugDraw")
        .constructor()
        .constructor<const b2DebugDraw&>()
        .property("drawingBounds", &b2DebugDraw::drawingBounds)
        .property("useDrawingBounds", &b2DebugDraw::useDrawingBounds)
        .property("drawShapes", &b2DebugDraw::drawShapes)
        .property("drawJoints", &b2DebugDraw::drawJoints)
        .property("drawJointExtras", &b2DebugDraw::drawJointExtras)
        .property("drawAABBs", &b2DebugDraw::drawAABBs)
        .property("drawMass", &b2DebugDraw::drawMass)
        .property("drawContacts", &b2DebugDraw::drawContacts)
        .property("drawGraphColors", &b2DebugDraw::drawGraphColors)
        .property("drawContactNormals", &b2DebugDraw::drawContactNormals)
        .property("drawContactImpulses", &b2DebugDraw::drawContactImpulses)
        .property("drawFrictionImpulses", &b2DebugDraw::drawFrictionImpulses)
    ;

    function("b2DefaultDebugDraw", &b2DefaultDebugDraw);
}

uint32_t g_seed = 12345;
uint32_t RAND_LIMIT = 32767;

// Simple random number generator. Using this instead of rand() for cross platform determinism.
B2_INLINE int RandomInt()
{
	// XorShift32 algorithm
	uint32_t x = g_seed;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	g_seed = x;

	// Map the 32-bit value to the range 0 to RAND_LIMIT
	return (int)( x % ( RAND_LIMIT + 1 ) );
}

// Random integer in range [lo, hi]
B2_INLINE float RandomIntRange( int lo, int hi )
{
	return lo + RandomInt() % ( hi - lo + 1 );
}

// Random number in range [-1,1]
B2_INLINE float RandomFloat()
{
	float r = (float)( RandomInt() & ( RAND_LIMIT ) );
	r /= RAND_LIMIT;
	r = 2.0f * r - 1.0f;
	return r;
}

// Random floating point number in range [lo, hi]
B2_INLINE float RandomFloatRange( float lo, float hi )
{
	float r = (float)( RandomInt() & ( RAND_LIMIT ) );
	r /= RAND_LIMIT;
	r = ( hi - lo ) * r + lo;
	return r;
}

// Random vector with coordinates in range [lo, hi]
B2_INLINE b2Vec2 RandomVec2( float lo, float hi )
{
	b2Vec2 v;
	v.x = RandomFloatRange( lo, hi );
	v.y = RandomFloatRange( lo, hi );
	return v;
}

EMSCRIPTEN_BINDINGS(box2d) {
    // ------------------------------------------------------------------------
    // b2World
    // ------------------------------------------------------------------------
    function("b2DefaultWorldDef", &b2DefaultWorldDef);

    function("b2CreateWorld", &b2CreateWorld, allow_raw_pointers());
    function("b2DestroyWorld", &b2DestroyWorld);
    function("b2World_Step", &b2World_Step);
    function("b2World_Draw", &b2World_Draw, allow_raw_pointers());
    function("b2World_GetBodyEvents", &b2World_GetBodyEvents);
    function("b2World_GetSensorEvents", &b2World_GetSensorEvents);
    function("b2World_GetContactEvents", &b2World_GetContactEvents);
    function("b2World_EnableSleeping", &b2World_EnableSleeping);
    function("b2World_IsSleepingEnabled", &b2World_IsSleepingEnabled);
    function("b2World_EnableWarmStarting", &b2World_EnableWarmStarting);
    function("b2World_IsWarmStartingEnabled", &b2World_IsWarmStartingEnabled);
    function("b2World_GetAwakeBodyCount", &b2World_GetAwakeBodyCount);
    function("b2World_EnableContinuous", &b2World_EnableContinuous);
    function("b2World_IsContinuousEnabled", &b2World_IsContinuousEnabled);
    function("b2World_SetRestitutionThreshold", &b2World_SetRestitutionThreshold);
    function("b2World_GetRestitutionThreshold", &b2World_GetRestitutionThreshold);
    function("b2World_SetHitEventThreshold", &b2World_SetHitEventThreshold);
    function("b2World_GetHitEventThreshold", &b2World_GetHitEventThreshold);
    function("b2World_SetContactTuning", &b2World_SetContactTuning);
    function("b2World_SetJointTuning", &b2World_SetJointTuning);
    function("b2World_SetMaximumLinearSpeed", &b2World_SetMaximumLinearSpeed);
    function("b2World_GetMaximumLinearSpeed", &b2World_GetMaximumLinearSpeed);
    function("b2World_GetProfile", &b2World_GetProfile);
    function("b2World_GetCounters", &b2World_GetCounters);
    function("b2World_GetPointer", +[](b2WorldId worldId) -> emscripten::val {
        return emscripten::val(worldId.index1);
    });
    // function("b2World_DumpMemoryStats", &b2World_DumpMemoryStats);
    function("b2World_OverlapAABB",
        +[](b2WorldId worldId, const b2AABB& aabb, b2QueryFilter filter, emscripten::val callback) {
            return b2World_OverlapAABB(worldId, aabb, filter,
                +[](b2ShapeId shapeId, void* ctx) -> bool {
                    return (*reinterpret_cast<emscripten::val*>(ctx))(emscripten::val(shapeId)).as<bool>();
                },
                &callback
            );
        }
    );
    function("b2World_OverlapPoint",
        +[](b2WorldId worldId, b2Vec2 point, b2Transform transform, b2QueryFilter filter, emscripten::val callback) {
            return b2World_OverlapPoint(worldId, point, transform, filter,
                +[](b2ShapeId shapeId, void* ctx) -> bool {
                    return (*reinterpret_cast<emscripten::val*>(ctx))(emscripten::val(shapeId)).as<bool>();
                },
                &callback
            );
        }
    );

    function("b2World_OverlapCircle",
        +[](b2WorldId worldId, const b2Circle* circle, b2Transform transform, b2QueryFilter filter, emscripten::val callback) {
            return b2World_OverlapCircle(worldId, circle, transform, filter,
                +[](b2ShapeId shapeId, void* ctx) -> bool {
                    return (*reinterpret_cast<emscripten::val*>(ctx))(emscripten::val(shapeId)).as<bool>();
                },
                &callback
            );
        }
    , allow_raw_pointers());

    function("b2World_OverlapCapsule",
        +[](b2WorldId worldId, const b2Capsule* capsule, b2Transform transform, b2QueryFilter filter, emscripten::val callback) {
            return b2World_OverlapCapsule(worldId, capsule, transform, filter,
                +[](b2ShapeId shapeId, void* ctx) -> bool {
                    return (*reinterpret_cast<emscripten::val*>(ctx))(emscripten::val(shapeId)).as<bool>();
                },
                &callback
            );
        }
    , allow_raw_pointers());

    function("b2World_OverlapPolygon",
        +[](b2WorldId worldId, const b2Polygon* polygon, b2Transform transform, b2QueryFilter filter, emscripten::val callback) {
            return b2World_OverlapPolygon(worldId, polygon, transform, filter,
                +[](b2ShapeId shapeId, void* ctx) -> bool {
                    return (*reinterpret_cast<emscripten::val*>(ctx))(emscripten::val(shapeId)).as<bool>();
                },
                &callback
            );
        }
    , allow_raw_pointers());

    function("b2World_CastRay",
        +[](b2WorldId worldId, b2Vec2 origin, b2Vec2 translation, b2QueryFilter filter, emscripten::val callback) {
            return b2World_CastRay(worldId, origin, translation, filter,
                +[](b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* userData) -> float {
                    auto callback = *reinterpret_cast<emscripten::val*>(userData);
                    return callback(
                        emscripten::val(shapeId),
                        emscripten::val(point),
                        emscripten::val(normal),
                        fraction
                    ).as<float>();
                },
                &callback
            );
        }
    );
    function("b2World_CastRayClosest", &b2World_CastRayClosest);
    function("b2World_CastCircle",
       +[](b2WorldId worldId, const b2Circle* circle, b2Transform originTransform, b2Vec2 translation, b2QueryFilter filter, emscripten::val callback) {
           return b2World_CastCircle(worldId, circle, originTransform, translation, filter,
               +[](b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* userData) -> float {
                   auto callback = *reinterpret_cast<emscripten::val*>(userData);
                   return callback(
                       emscripten::val(shapeId),
                       emscripten::val(point),
                       emscripten::val(normal),
                       fraction
                   ).as<float>();
               },
               &callback
           );
       }
    , allow_raw_pointers());
    function("b2World_CastCapsule",
       +[](b2WorldId worldId, const b2Capsule* capsule, b2Transform originTransform, b2Vec2 translation, b2QueryFilter filter, emscripten::val callback) {
           return b2World_CastCapsule(worldId, capsule, originTransform, translation, filter,
               +[](b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* userData) -> float {
                   auto callback = *reinterpret_cast<emscripten::val*>(userData);
                   return callback(
                       emscripten::val(shapeId),
                       emscripten::val(point),
                       emscripten::val(normal),
                       fraction
                   ).as<float>();
               },
               &callback
           );
       }
    , allow_raw_pointers());
    function("b2World_CastPolygon",
       +[](b2WorldId worldId, const b2Polygon* polygon, b2Transform originTransform, b2Vec2 translation, b2QueryFilter filter, emscripten::val callback) {
           return b2World_CastPolygon(worldId, polygon, originTransform, translation, filter,
               +[](b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* userData) -> float {
                   auto callback = *reinterpret_cast<emscripten::val*>(userData);
                   return callback(
                       emscripten::val(shapeId),
                       emscripten::val(point),
                       emscripten::val(normal),
                       fraction
                   ).as<float>();
               },
               &callback
           );
       }
    , allow_raw_pointers());
    function("b2World_SetCustomFilterCallback",
       +[](b2WorldId worldId, emscripten::val callback) {
           b2World_SetCustomFilterCallback(worldId,
               +[](b2ShapeId shapeIdA, b2ShapeId shapeIdB, void* userData) -> bool {
                   auto callback = *reinterpret_cast<emscripten::val*>(userData);
                   return callback(
                       emscripten::val(shapeIdA),
                       emscripten::val(shapeIdB)
                   ).as<bool>();
               },
               new emscripten::val(callback)
           );
       }
    );
    function("b2World_SetPreSolveCallback",
       +[](b2WorldId worldId, emscripten::val callback) {
           b2World_SetPreSolveCallback(worldId,
               +[](b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, void* userData) -> bool {
                   auto callback = *reinterpret_cast<emscripten::val*>(userData);
                   return callback(
                       emscripten::val(shapeIdA),
                       emscripten::val(shapeIdB),
                       emscripten::val(manifold)
                   ).as<bool>();
               },
               new emscripten::val(callback)
           );
       }
    );
    function("b2World_SetGravity", &b2World_SetGravity);
    function("b2World_GetGravity", &b2World_GetGravity);
    function("b2World_Explode", &b2World_Explode, allow_raw_pointers());
    function("b2World_RebuildStaticTree", &b2World_RebuildStaticTree);
    function("b2World_EnableSpeculative", &b2World_EnableSpeculative);

    // ------------------------------------------------------------------------
    // b2Shape
    // ------------------------------------------------------------------------
    function("b2Shape_GetBody", &b2Shape_GetBody);
    function("b2Shape_GetWorld", &b2Shape_GetWorld);
    function("b2Shape_GetPointer", +[](b2ShapeId shapeId) -> emscripten::val {
        return emscripten::val(shapeId.index1);
    });
    function("b2Shape_IsValid", &b2Shape_IsValid);
    function("b2Shape_IsSensor", &b2Shape_IsSensor);
    function("b2Shape_TestPoint", &b2Shape_TestPoint);
    function("b2Shape_RayCast", &b2Shape_RayCast, allow_raw_pointers());
    function("b2Shape_SetDensity", &b2Shape_SetDensity);
    function("b2Shape_GetDensity", &b2Shape_GetDensity);
    function("b2Shape_SetFriction", &b2Shape_SetFriction);
    function("b2Shape_GetFriction", &b2Shape_GetFriction);
    function("b2Shape_SetRestitution", &b2Shape_SetRestitution);
    function("b2Shape_GetRestitution", &b2Shape_GetRestitution);
    function("b2Shape_GetFilter", &b2Shape_GetFilter);
    function("b2Shape_SetFilter", &b2Shape_SetFilter);
    function("b2Shape_EnableSensorEvents", &b2Shape_EnableSensorEvents);
    function("b2Shape_AreSensorEventsEnabled", &b2Shape_AreSensorEventsEnabled);
    function("b2Shape_EnableContactEvents", &b2Shape_EnableContactEvents);
    function("b2Shape_AreContactEventsEnabled", &b2Shape_AreContactEventsEnabled);
    function("b2Shape_EnablePreSolveEvents", &b2Shape_EnablePreSolveEvents);
    function("b2Shape_ArePreSolveEventsEnabled", &b2Shape_ArePreSolveEventsEnabled);
    function("b2Shape_EnableHitEvents", &b2Shape_EnableHitEvents);
    function("b2Shape_AreHitEventsEnabled", &b2Shape_AreHitEventsEnabled);
    function("b2Shape_GetType", &b2Shape_GetType);
    function("b2Shape_GetCircle", &b2Shape_GetCircle);
    function("b2Shape_GetSegment", &b2Shape_GetSegment);
    function("b2Shape_GetChainSegment", &b2Shape_GetChainSegment);
    function("b2Shape_GetCapsule", &b2Shape_GetCapsule);
    function("b2Shape_GetPolygon", &b2Shape_GetPolygon);
    function("b2Shape_SetCircle", &b2Shape_SetCircle, allow_raw_pointers());
    function("b2Shape_SetCapsule", &b2Shape_SetCapsule, allow_raw_pointers());
    function("b2Shape_SetSegment", &b2Shape_SetSegment, allow_raw_pointers());
    function("b2Shape_SetPolygon", &b2Shape_SetPolygon, allow_raw_pointers());
    function("b2Shape_GetParentChain", &b2Shape_GetParentChain);
    function("b2Shape_GetContactCapacity", &b2Shape_GetContactCapacity);
    function("b2Shape_GetContactData", +[](b2ShapeId shapeId, int capacity) -> emscripten::val {
        std::vector<b2ContactData> contactData(capacity);
        int count = b2Shape_GetContactData(shapeId, contactData.data(), capacity);
        auto result = emscripten::val::array();
        for (int i = 0; i < count; i++) {
            result.set(i, contactData[i]);
        }
        return result;
    }, allow_raw_pointers());
    function("b2Shape_GetSensorCapacity", &b2Shape_GetSensorCapacity);
    function("b2Shape_GetSensorOverlaps", +[](b2ShapeId shapeId, int capacity) -> emscripten::val {
        std::vector<b2ShapeId> overlappedShapes(capacity);
        int count = b2Shape_GetSensorOverlaps(shapeId, overlappedShapes.data(), capacity);
        auto result = emscripten::val::array();
        for (int i = 0; i < count; i++) {
            result.set(i, overlappedShapes[i]);
        }
        return result;
    }, allow_raw_pointers());
    function("b2Shape_GetAABB", &b2Shape_GetAABB);
    function("b2Shape_GetClosestPoint", &b2Shape_GetClosestPoint);

    // ------------------------------------------------------------------------
    // b2Body
    // ------------------------------------------------------------------------
    function("b2DefaultBodyDef", &b2DefaultBodyDef);

    function("b2CreateBody", &b2CreateBody, allow_raw_pointers());
    function("b2DestroyBody", &b2DestroyBody, allow_raw_pointers());
    function("b2Body_GetPosition", &b2Body_GetPosition);
    function("b2Body_GetRotation", &b2Body_GetRotation);
    function("b2Body_GetTransform", &b2Body_GetTransform);
    function("b2Body_SetTransform", &b2Body_SetTransform);
    function("b2Body_GetLocalPoint", &b2Body_GetLocalPoint);
    function("b2Body_GetWorldPoint", &b2Body_GetWorldPoint);
    function("b2Body_GetLocalVector", &b2Body_GetLocalVector);
    function("b2Body_GetWorldVector", &b2Body_GetWorldVector);
    function("b2Body_GetMass", &b2Body_GetMass);
    function("b2Body_GetRotationalInertia", &b2Body_GetRotationalInertia);
    function("b2Body_GetLocalCenterOfMass", &b2Body_GetLocalCenterOfMass);
    function("b2Body_GetWorldCenterOfMass", &b2Body_GetWorldCenterOfMass);
    function("b2Body_SetMassData", &b2Body_SetMassData);
    function("b2Body_GetMassData", &b2Body_GetMassData);
    function("b2Body_ApplyMassFromShapes", &b2Body_ApplyMassFromShapes);
    function("b2Body_GetLinearVelocity", &b2Body_GetLinearVelocity);
    function("b2Body_GetAngularVelocity", &b2Body_GetAngularVelocity);
    function("b2Body_SetLinearVelocity", &b2Body_SetLinearVelocity);
    function("b2Body_SetAngularVelocity", &b2Body_SetAngularVelocity);
    function("b2Body_ApplyForce", &b2Body_ApplyForce);
    function("b2Body_ApplyForceToCenter", &b2Body_ApplyForceToCenter);
    function("b2Body_ApplyTorque", &b2Body_ApplyTorque);
    function("b2Body_ApplyLinearImpulse", &b2Body_ApplyLinearImpulse);
    function("b2Body_ApplyLinearImpulseToCenter", &b2Body_ApplyLinearImpulseToCenter);
    function("b2Body_ApplyAngularImpulse", &b2Body_ApplyAngularImpulse);
    function("b2Body_SetLinearDamping", &b2Body_SetLinearDamping);
    function("b2Body_GetLinearDamping", &b2Body_GetLinearDamping);
    function("b2Body_SetAngularDamping", &b2Body_SetAngularDamping);
    function("b2Body_GetAngularDamping", &b2Body_GetAngularDamping);
    function("b2Body_SetGravityScale", &b2Body_SetGravityScale);
    function("b2Body_GetGravityScale", &b2Body_GetGravityScale);
    function("b2Body_GetType", &b2Body_GetType);
    function("b2Body_SetType", &b2Body_SetType);
    function("b2Body_IsAwake", &b2Body_IsAwake);
    function("b2Body_SetAwake", &b2Body_SetAwake);
    function("b2Body_IsEnabled", &b2Body_IsEnabled);
    function("b2Body_Enable", &b2Body_Enable);
    function("b2Body_Disable", &b2Body_Disable);
    function("b2Body_IsSleepEnabled", &b2Body_IsSleepEnabled);
    function("b2Body_EnableSleep", &b2Body_EnableSleep);
    function("b2Body_SetSleepThreshold", &b2Body_SetSleepThreshold);
    function("b2Body_GetSleepThreshold", &b2Body_GetSleepThreshold);
    function("b2Body_SetFixedRotation", &b2Body_SetFixedRotation);
    function("b2Body_IsFixedRotation", &b2Body_IsFixedRotation);
    function("b2Body_SetBullet", &b2Body_SetBullet);
    function("b2Body_IsBullet", &b2Body_IsBullet);
    function("b2Body_GetPointer", +[](b2BodyId bodyId) -> emscripten::val {
        return emscripten::val(bodyId.index1);
    });
    function("b2Body_EnableSensorEvents", &b2Body_EnableSensorEvents);
    function("b2Body_EnableContactEvents", &b2Body_EnableContactEvents);
    function("b2Body_EnableHitEvents", &b2Body_EnableHitEvents);
    function("b2Body_GetShapeCount", &b2Body_GetShapeCount);
    function("b2Body_GetShapes", +[](b2BodyId bodyId, int capacity) -> emscripten::val {
        std::vector<b2ShapeId> shapes(capacity);
        int count = b2Body_GetShapes(bodyId, shapes.data(), capacity);

        auto result = emscripten::val::array();
        for (int i = 0; i < count; i++) {
            result.set(i, shapes[i]);
        }
        return result;
    });
    function("b2Body_GetJointCount", &b2Body_GetJointCount);
    function("b2Body_GetJoints", &b2Body_GetJoints, allow_raw_pointers());
    function("b2Body_GetContactCapacity", &b2Body_GetContactCapacity);
    function("b2Body_GetContactData", +[](b2BodyId bodyId, int capacity) -> emscripten::val {
        std::vector<b2ContactData> contactData(capacity);
        int count = b2Body_GetContactData(bodyId, contactData.data(), capacity);
        auto result = emscripten::val::array();
        for (int i = 0; i < count; i++) {
            result.set(i, contactData[i]);
        }
        return result;
    }, allow_raw_pointers());
    function("b2Body_ComputeAABB", &b2Body_ComputeAABB);
    function("b2Body_GetWorld", &b2Body_GetWorld);

    // ------------------------------------------------------------------------
    // b2Joint
    // ------------------------------------------------------------------------
    function("b2DestroyJoint", &b2DestroyJoint, allow_raw_pointers());
    function("b2DefaultDistanceJointDef", &b2DefaultDistanceJointDef);
    function("b2CreateDistanceJoint", &b2CreateDistanceJoint, allow_raw_pointers());
    function("b2DefaultMotorJointDef", &b2DefaultMotorJointDef);
    function("b2CreateMotorJoint", &b2CreateMotorJoint, allow_raw_pointers());
    function("b2DefaultMouseJointDef", &b2DefaultMouseJointDef);
    function("b2CreateMouseJoint", &b2CreateMouseJoint, allow_raw_pointers());
    function("b2DefaultNullJointDef", &b2DefaultNullJointDef);
    function("b2CreateNullJoint", &b2CreateNullJoint, allow_raw_pointers());
    function("b2DefaultPrismaticJointDef", &b2DefaultPrismaticJointDef);
    function("b2CreatePrismaticJoint", &b2CreatePrismaticJoint, allow_raw_pointers());
    function("b2DefaultRevoluteJointDef", &b2DefaultRevoluteJointDef);
    function("b2CreateRevoluteJoint", &b2CreateRevoluteJoint, allow_raw_pointers());
    function("b2DefaultWeldJointDef", &b2DefaultWeldJointDef);
    function("b2CreateWeldJoint", &b2CreateWeldJoint, allow_raw_pointers());
    function("b2DefaultWheelJointDef", &b2DefaultWheelJointDef);
    function("b2CreateWheelJoint", &b2CreateWheelJoint, allow_raw_pointers());

    function("b2Joint_IsValid", &b2Joint_IsValid);
    function("b2Joint_GetType", &b2Joint_GetType);
    function("b2Joint_GetBodyA", &b2Joint_GetBodyA);
    function("b2Joint_GetBodyB", &b2Joint_GetBodyB);
    function("b2Joint_GetWorld", &b2Joint_GetWorld);
    function("b2Joint_GetLocalAnchorA", &b2Joint_GetLocalAnchorA);
    function("b2Joint_GetLocalAnchorB", &b2Joint_GetLocalAnchorB);
    function("b2Joint_SetCollideConnected", &b2Joint_SetCollideConnected);
    function("b2Joint_GetCollideConnected", &b2Joint_GetCollideConnected);
    function("b2Joint_GetPointer", +[](b2JointId jointId) -> emscripten::val {
        return emscripten::val(jointId.index1);
    });
    function("b2Joint_WakeBodies", &b2Joint_WakeBodies);
    function("b2Joint_GetConstraintForce", &b2Joint_GetConstraintForce);
    function("b2Joint_GetConstraintTorque", &b2Joint_GetConstraintTorque);

    function("b2MouseJoint_SetTarget", &b2MouseJoint_SetTarget);
    function("b2MouseJoint_GetTarget", &b2MouseJoint_GetTarget);
    function("b2MouseJoint_SetSpringHertz", &b2MouseJoint_SetSpringHertz);
    function("b2MouseJoint_GetSpringHertz", &b2MouseJoint_GetSpringHertz);
    function("b2MouseJoint_SetSpringDampingRatio", &b2MouseJoint_SetSpringDampingRatio);
    function("b2MouseJoint_GetSpringDampingRatio", &b2MouseJoint_GetSpringDampingRatio);
    function("b2MouseJoint_SetMaxForce", &b2MouseJoint_SetMaxForce);
    function("b2MouseJoint_GetMaxForce", &b2MouseJoint_GetMaxForce);

    // ------------------------------------------------------------------------
    // Misc  
    // ------------------------------------------------------------------------
    // PAUMOD, added _count parameter becuase there was a strange error
    function("b2ComputeHull", optional_override([](const emscripten::val& pointsArray) -> b2Hull {
        int count = pointsArray["length"].as<int>();
        std::vector<b2Vec2> points(count);
        for (int i = 0; i < count; i++) {
            const auto point = pointsArray[i];
            points[i].x = point["x"].as<float>();
            points[i].y = point["y"].as<float>();
        }
        b2Hull hh = b2ComputeHull(points.data(), count);
        return hh;
    }));

    function("B2_ID_EQUALS", +[](const emscripten::val& id1, const emscripten::val& id2) -> bool {
        return  id1["index1"].as<int32_t>() == id2["index1"].as<int32_t>() &&
                id1["world0"].as<uint16_t>() == id2["world0"].as<uint16_t>() &&
                id1["revision"].as<uint16_t>() == id2["revision"].as<uint16_t>();
    });

    constant("B2_PI", B2_PI);
    function("b2Dot", &b2Dot);
    function("b2Cross", &b2Cross);
    function("b2CrossVS", &b2CrossVS);
    function("b2CrossSV", &b2CrossSV);
    function("b2LeftPerp", &b2LeftPerp);
    function("b2RightPerp", &b2RightPerp);
    function("b2Add", &b2Add);
    function("b2Sub", &b2Sub);
    function("b2Neg", &b2Neg);
    function("b2Lerp", &b2Lerp);
    function("b2Mul", &b2Mul);
    function("b2MulSV", &b2MulSV);
    function("b2MulAdd", &b2MulAdd);
    function("b2MulSub", &b2MulSub);
    function("b2Abs", &b2Abs);
    function("b2Min", &b2Min);
    function("b2Max", &b2Max);
    function("b2Clamp", &b2Clamp);
    function("b2Length", &b2Length);
    function("b2Distance", &b2Distance);
    function("b2Normalize", &b2Normalize);
    function("b2GetLengthAndNormalize", +[](b2Vec2 v) {
        float length;
        return b2GetLengthAndNormalize(&length, v);
    });
    function("b2NormalizeRot", &b2NormalizeRot);
    function("b2IntegrateRotation", &b2IntegrateRotation);
    function("b2LengthSquared", &b2LengthSquared);
    function("b2DistanceSquared", &b2DistanceSquared);
    function("b2MakeRot", &b2MakeRot);
    function("b2IsNormalized", &b2IsNormalized);
    function("b2NLerp", &b2NLerp);
    function("b2ComputeAngularVelocity", &b2ComputeAngularVelocity);
    function("b2Rot_GetAngle", &b2Rot_GetAngle);
    function("b2Rot_GetXAxis", &b2Rot_GetXAxis);
    function("b2Rot_GetYAxis", &b2Rot_GetYAxis);
    function("b2MulRot", &b2MulRot);
    function("b2InvMulRot", &b2InvMulRot);
    function("b2RelativeAngle", &b2RelativeAngle);
    function("b2UnwindAngle", &b2UnwindAngle);
    function("b2UnwindLargeAngle", &b2UnwindLargeAngle);
    function("b2RotateVector", &b2RotateVector);
    function("b2InvRotateVector", &b2InvRotateVector);
    function("b2TransformPoint", &b2TransformPoint);
    function("b2InvTransformPoint", &b2InvTransformPoint);
    function("b2MulTransforms", &b2MulTransforms);
    function("b2InvMulTransforms", &b2InvMulTransforms);
    function("b2MulMV", &b2MulMV);
    function("b2GetInverse22", &b2GetInverse22);
    function("b2Solve22", &b2Solve22);
    function("b2AABB_Contains", &b2AABB_Contains);
    function("b2AABB_Center", &b2AABB_Center);
    function("b2AABB_Extents", &b2AABB_Extents);
    function("b2AABB_Union", &b2AABB_Union);
    function("b2Hash", +[](uint32_t hash, const emscripten::val& array) -> uint32_t {
        if (!array.instanceof(emscripten::val::global("Uint8Array"))) {
            throw std::runtime_error("Expected a Uint8Array.");
        }
        auto buffer = array["buffer"].as<emscripten::val>();
        size_t length = array["length"].as<size_t>();
        int byteOffset = array["byteOffset"].as<int>();
        auto view = emscripten::val::global("Uint8Array").new_(buffer, byteOffset, length);
        uint32_t result = hash;
        for (size_t i = 0; i < length; i++) {
            result = (result << 5) + result + view[i].as<uint8_t>();
        }
        return result;
    }, allow_raw_pointers());
    constant("B2_HASH_INIT", B2_HASH_INIT);

    // ------------------------------------------------------------------------
    // Random
    // ------------------------------------------------------------------------
    function("RandomInt", &RandomInt);
    function("RandomIntRange", &RandomIntRange);
    function("RandomFloat", &RandomFloat);
    function("RandomFloatRange", &RandomFloatRange);
    function("RandomVec2", &RandomVec2);
    function("RandomPolygon", optional_override([](float extent) -> b2Polygon {
        b2Vec2 points[B2_MAX_POLYGON_VERTICES];
        int count = 3 + RandomInt() % 6;
        for (int i = 0; i < count; ++i) {
            points[i] = RandomVec2(-extent, extent);
        }

        b2Hull hull = b2ComputeHull(points, count);
        if (hull.count > 0) {
            return b2MakePolygon(&hull, 0.0f);
        }

        return b2MakeSquare(extent);
    }));
}
