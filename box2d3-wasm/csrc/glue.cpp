#include <box2d/box2d.h>
#include <box2cpp/box2cpp.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
using namespace b2;

// EMSCRIPTEN_DECLARE_VAL_TYPE(b2TaskCallback)
// EMSCRIPTEN_DECLARE_VAL_TYPE(b2EnqueueTaskCallback)

EMSCRIPTEN_BINDINGS(box2d) {
    class_<b2Vec2>("b2Vec2")
        .constructor()
        .property("x", &b2Vec2::x)
        .property("y", &b2Vec2::y)
        ;
    
    class_<b2CosSin>("b2CosSin")
        .constructor()
        .property("cosine", &b2CosSin::cosine)
        .property("sine", &b2CosSin::sine)
        ;
    
    class_<b2Rot>("b2Rot")
        .constructor()
        .property("c", &b2Rot::c)
        .property("s", &b2Rot::s)
        ;

    class_<b2Transform>("b2Transform")
        .constructor()
        .property("p", &b2Transform::p)
        .property("q", &b2Transform::q)
        ;

    class_<b2Mat22>("b2Mat22")
        .constructor()
        .property("cx", &b2Mat22::cx)
        .property("cy", &b2Mat22::cy)
        ;
    
    class_<b2AABB>("b2AABB")
        .constructor()
        .property("lowerBound", &b2AABB::lowerBound)
        .property("upperBound", &b2AABB::upperBound)
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
        .property("gravity", &b2WorldDef::gravity)
        .property("restitutionThreshold", &b2WorldDef::restitutionThreshold)
        .property("contactPushoutVelocity", &b2WorldDef::contactPushoutVelocity)
        .property("hitEventThreshold", &b2WorldDef::hitEventThreshold)
        .property("contactHertz", &b2WorldDef::contactHertz)
        .property("contactDampingRatio", &b2WorldDef::contactDampingRatio)
        .property("jointHertz", &b2WorldDef::jointHertz)
        .property("jointDampingRatio", &b2WorldDef::jointDampingRatio)
        .property("maximumLinearVelocity", &b2WorldDef::maximumLinearVelocity)
        .property("frictionMixingRule", &b2WorldDef::frictionMixingRule)
        .property("restitutionMixingRule", &b2WorldDef::restitutionMixingRule)
        .property("enableSleep", &b2WorldDef::enableSleep)
        .property("enableContinuous", &b2WorldDef::enableContinuous)
        .property("workerCount", &b2WorldDef::workerCount)
        // .property("enqueueTask", &b2WorldDef::enqueueTask, allow_raw_pointers())
        // .property("finishTask", &b2WorldDef::finishTask, allow_raw_pointers())
        // .property("userTaskContext", &b2WorldDef::userTaskContext, allow_raw_pointers())
        // .property("userData", &b2WorldDef::userData, allow_raw_pointers())
        .property("internalValue", &b2WorldDef::internalValue)
        ;

    class_<b2::World>("World")
        .constructor()
        .constructor<b2WorldDef&>()
        // .function("World", select_overload<b2::World()>(&b2::World))
        // .function("World", &World::World)
        ;
}