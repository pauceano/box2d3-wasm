#include "CanvasDebugDraw.h"

CanvasDebugDraw::CanvasDebugDraw(emscripten::val ctx) : context(ctx) {
    callbacks.context = this;

    callbacks.drawShapes = true;
    callbacks.drawJoints = true;
    callbacks.useDrawingBounds = false;

    callbacks.DrawPolygon = [](const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");

        for (int i = 0; i < vertexCount; ++i) {
            const auto& v = vertices[i];
            if (i == 0) {
                self->context.call<void>("moveTo", v.x, v.y);
            } else {
                self->context.call<void>("lineTo", v.x, v.y);
            }
        }

        self->context.call<void>("closePath");
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");
    };

    callbacks.DrawSolidPolygon = [](b2Transform xf, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");

        for (int i = 0; i < vertexCount; ++i) {
            b2Vec2 v = b2TransformPoint(xf, vertices[i]);
            if (i == 0) {
                self->context.call<void>("moveTo", v.x, v.y);
            } else {
                self->context.call<void>("lineTo", v.x, v.y);
            }
        }

        self->context.call<void>("closePath");
        self->context.set("fillStyle", ColorToHTML(color, 0.5f));
        self->context.call<void>("fill");
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");
    };

    callbacks.DrawCircle = [](b2Vec2 center, float radius, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");
        self->context.call<void>("arc", center.x, center.y, radius, 0, 2 * M_PI);
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");
    };

    callbacks.DrawSolidCircle = [](b2Transform xf, float radius, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");
        self->context.call<void>("arc", xf.p.x, xf.p.y, radius, 0, 2 * M_PI);
        self->context.set("fillStyle", ColorToHTML(color, 0.5f));
        self->context.call<void>("fill");
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");

        b2Vec2 axis = b2Rot_GetXAxis(xf.q);
        b2Vec2 p2 = b2MulAdd(xf.p, radius, axis);
        self->context.call<void>("beginPath");
        self->context.call<void>("moveTo", xf.p.x, xf.p.y);
        self->context.call<void>("lineTo", p2.x, p2.y);
        self->context.call<void>("stroke");
    };

    callbacks.DrawSolidCapsule = [](b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);

        b2Vec2 d = b2Sub(p2, p1);
        float length = b2Length(d);
        if (length < 0.001f) {
            return;
        }

        b2Vec2 axis = {d.x / length, d.y / length};
        float angle = atan2(axis.y, axis.x);

        self->context.call<void>("beginPath");
        self->context.call<void>("moveTo", p1.x + radius * axis.y, p1.y - radius * axis.x);
        self->context.call<void>("lineTo", p2.x + radius * axis.y, p2.y - radius * axis.x);
        self->context.call<void>("arc", p2.x, p2.y, radius, angle - M_PI/2, angle + M_PI/2);
        self->context.call<void>("lineTo", p1.x - radius * axis.y, p1.y + radius * axis.x);
        self->context.call<void>("arc", p1.x, p1.y, radius, angle + M_PI/2, angle + 3*M_PI/2);
        self->context.call<void>("closePath");
        self->context.set("fillStyle", ColorToHTML(color, 0.5f));
        self->context.call<void>("fill");
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");
    };

    callbacks.DrawSegment = [](b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");
        self->context.call<void>("moveTo", p1.x, p1.y);
        self->context.call<void>("lineTo", p2.x, p2.y);
        self->context.set("strokeStyle", ColorToHTML(color));
        self->context.call<void>("stroke");
    };

    callbacks.DrawTransform = [](b2Transform xf, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        const float k_axisScale = 0.2f;

        b2Vec2 p1 = xf.p;
        b2Vec2 p2 = b2MulAdd(p1, k_axisScale, b2Rot_GetXAxis(xf.q));
        self->context.call<void>("beginPath");
        self->context.call<void>("moveTo", p1.x, p1.y);
        self->context.call<void>("lineTo", p2.x, p2.y);
        self->context.set("strokeStyle", ColorToHTML(b2_colorRed));
        self->context.call<void>("stroke");

        p2 = b2MulAdd(p1, k_axisScale, b2Rot_GetYAxis(xf.q));
        self->context.call<void>("beginPath");
        self->context.call<void>("moveTo", p1.x, p1.y);
        self->context.call<void>("lineTo", p2.x, p2.y);
        self->context.set("strokeStyle", ColorToHTML(b2_colorGreen));
        self->context.call<void>("stroke");
    };

    callbacks.DrawPoint = [](b2Vec2 p, float size, b2HexColor color, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.call<void>("beginPath");
        self->context.call<void>("arc", p.x, p.y, size/2, 0, 2 * M_PI);
        self->context.set("fillStyle", ColorToHTML(color));
        self->context.call<void>("fill");
    };

    callbacks.DrawString = [](b2Vec2 p, const char* string, void* context) {
        auto* self = static_cast<CanvasDebugDraw*>(context);
        self->context.set("fillStyle", "rgb(230, 230, 230)");
        self->context.call<void>("fillText", std::string(string), p.x, p.y);
    };
}

std::string CanvasDebugDraw::ColorToHTML(b2HexColor color, float alpha) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    return "rgba(" + std::to_string(r) + "," +
                    std::to_string(g) + "," +
                    std::to_string(b) + "," +
                    std::to_string(alpha) + ")";
}
