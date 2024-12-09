#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace b2;

// EMSCRIPTEN_DECLARE_VAL_TYPE(b2TaskCallback)
// EMSCRIPTEN_DECLARE_VAL_TYPE(b2EnqueueTaskCallback)

EMSCRIPTEN_BINDINGS(box2d) {
    value_object<b2Vec2>("b2Vec2")
        .field("x", &b2Vec2::x)
        .field("y", &b2Vec2::y)
        ;
    
    value_object<b2CosSin>("b2CosSin")
        .field("cosine", &b2CosSin::cosine)
        .field("sine", &b2CosSin::sine)
        ;
    
    value_object<b2Rot>("b2Rot")
        .field("c", &b2Rot::c)
        .field("s", &b2Rot::s)
        ;

    value_object<b2Transform>("b2Transform")
        .field("p", &b2Transform::p)
        .field("q", &b2Transform::q)
        ;

    value_object<b2Mat22>("b2Mat22")
        .field("cx", &b2Mat22::cx)
        .field("cy", &b2Mat22::cy)
        ;
    
    value_object<b2AABB>("b2AABB")
        .field("lowerBound", &b2AABB::lowerBound)
        .field("upperBound", &b2AABB::upperBound)
        ;
    
    enum_<b2MixingRule>("b2MixingRule")
        .value("b2_mixAverage", b2MixingRule::b2_mixAverage)
        .value("b2_mixGeometricMean", b2MixingRule::b2_mixGeometricMean)
        .value("b2_mixMultiply", b2MixingRule::b2_mixMultiply)
        .value("b2_mixMinimum", b2MixingRule::b2_mixMinimum)
        .value("b2_mixMaximum", b2MixingRule::b2_mixMaximum)
        ;

    value_object<b2WorldDef>("b2WorldDef")
        .field("gravity", &b2WorldDef::gravity)
        .field("restitutionThreshold", &b2WorldDef::restitutionThreshold)
        .field("contactPushoutVelocity", &b2WorldDef::contactPushoutVelocity)
        .field("hitEventThreshold", &b2WorldDef::hitEventThreshold)
        .field("contactHertz", &b2WorldDef::contactHertz)
        .field("contactDampingRatio", &b2WorldDef::contactDampingRatio)
        .field("jointHertz", &b2WorldDef::jointHertz)
        .field("jointDampingRatio", &b2WorldDef::jointDampingRatio)
        .field("maximumLinearVelocity", &b2WorldDef::maximumLinearVelocity)
        .field("frictionMixingRule", &b2WorldDef::frictionMixingRule)
        .field("restitutionMixingRule", &b2WorldDef::restitutionMixingRule)
        .field("enableSleep", &b2WorldDef::enableSleep)
        .field("enableContinuous", &b2WorldDef::enableContinuous)
        .field("workerCount", &b2WorldDef::workerCount)
        // .field("enqueueTask", &b2WorldDef::enqueueTask, allow_raw_pointers())
        // .field("finishTask", &b2WorldDef::finishTask, allow_raw_pointers())
        // .field("userTaskContext", &b2WorldDef::userTaskContext, allow_raw_pointers())
        // .field("userData", &b2WorldDef::userData, allow_raw_pointers())
        .field("internalValue", &b2WorldDef::internalValue)
        ;

    class_<b2::World>("World")
        .constructor()
        .constructor<b2WorldDef&>()
        // .function("World", select_overload<b2::World()>(&b2::World))
        // .function("World", &World::World)
        ;
}