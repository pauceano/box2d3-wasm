#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <vector>
#include <box2d/box2d.h>

struct DrawCmd {
    uint8_t type;
    uint32_t color;
    uint16_t vertexCount;
    float data[32];
};

class CanvasDebugDraw {
public:
    CanvasDebugDraw(size_t maxCommands) : maxCommands(maxCommands) {
        debugDraw.context = this;
        commands.reserve(maxCommands);

        debugDraw.drawShapes = true;
        debugDraw.drawJoints = true;
        debugDraw.drawJointExtras = false;
        debugDraw.drawAABBs = false;
        debugDraw.drawMass = false;
        debugDraw.drawContacts = false;
        debugDraw.drawGraphColors = false;
        debugDraw.drawContactNormals = false;
        debugDraw.drawContactImpulses = false;
        debugDraw.drawFrictionImpulses = false;
        debugDraw.useDrawingBounds = false;

        debugDraw.DrawPolygon = [](const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 0;
            cmd.color = color;
            cmd.vertexCount = std::min((uint16_t)vertexCount, (uint16_t)16);

            for (int i = 0; i < cmd.vertexCount; i++) {
                cmd.data[i*2] = vertices[i].x;
                cmd.data[i*2+1] = vertices[i].y;
            }

            self->commands.push_back(cmd);
        };

        debugDraw.DrawSolidPolygon = [](b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 1;
            cmd.color = color;
            cmd.vertexCount = std::min((uint16_t)vertexCount, (uint16_t)14) + 2;

            cmd.data[0] = transform.p.x;
            cmd.data[1] = transform.p.y;
            cmd.data[2] = transform.q.s;
            cmd.data[3] = transform.q.c;

            for (int i = 0; i < cmd.vertexCount - 2; i++) {
                cmd.data[i*2 + 4] = vertices[i].x;
                cmd.data[i*2 + 5] = vertices[i].y;
            }

            self->commands.push_back(cmd);
        };

        debugDraw.DrawCircle = [](b2Vec2 center, float radius, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 2;
            cmd.color = color;
            cmd.vertexCount = 1;
            cmd.data[0] = center.x;
            cmd.data[1] = center.y;
            cmd.data[2] = radius;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawSolidCircle = [](b2Transform transform, float radius, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 3;
            cmd.color = color;
            cmd.vertexCount = 1;
            cmd.data[0] = transform.p.x;
            cmd.data[1] = transform.p.y;
            cmd.data[2] = transform.q.s;
            cmd.data[3] = transform.q.c;
            cmd.data[4] = radius;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawSolidCapsule = [](b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 4;
            cmd.color = color;
            cmd.vertexCount = 2;
            cmd.data[0] = p1.x;
            cmd.data[1] = p1.y;
            cmd.data[2] = p2.x;
            cmd.data[3] = p2.y;
            cmd.data[4] = radius;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawSegment = [](b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 5;
            cmd.color = color;
            cmd.vertexCount = 2;
            cmd.data[0] = p1.x;
            cmd.data[1] = p1.y;
            cmd.data[2] = p2.x;
            cmd.data[3] = p2.y;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawTransform = [](b2Transform transform, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 6;
            cmd.vertexCount = 1;
            cmd.data[0] = transform.p.x;
            cmd.data[1] = transform.p.y;
            cmd.data[2] = transform.q.s;
            cmd.data[3] = transform.q.c;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawPoint = [](b2Vec2 p, float size, b2HexColor color, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 7;
            cmd.color = color;
            cmd.vertexCount = 1;
            cmd.data[0] = p.x;
            cmd.data[1] = p.y;
            cmd.data[2] = size;

            self->commands.push_back(cmd);
        };

        debugDraw.DrawString = [](b2Vec2 p, const char* s, void* context) {
            auto* self = static_cast<CanvasDebugDraw*>(context);
            if (self->commands.size() >= self->maxCommands) return;

            DrawCmd cmd;
            cmd.type = 8;
            cmd.vertexCount = 1;
            cmd.data[0] = p.x;
            cmd.data[1] = p.y;
            // Skip string handling for now

            self->commands.push_back(cmd);
        };
    }

    b2DebugDraw* GetDebugDraw() { return &debugDraw; }
    void ClearCommands() { commands.clear(); }
    size_t GetMaxCommands() const { return maxCommands; }

    uintptr_t GetCommandsData() const {
        return reinterpret_cast<uintptr_t>(commands.data());
    }

    size_t GetCommandsSize() const {
        return commands.size();
    }

    size_t GetCommandStride() const {
        return sizeof(DrawCmd);
    }

private:
    b2DebugDraw debugDraw;
    std::vector<DrawCmd> commands;
    size_t maxCommands;
};

EMSCRIPTEN_BINDINGS(canvas_debug_draw) {
    emscripten::class_<CanvasDebugDraw>("CanvasDebugDraw")
        .constructor<size_t>()
        .function("GetDebugDraw", &CanvasDebugDraw::GetDebugDraw, emscripten::allow_raw_pointers())
        .function("GetCommandsData", &CanvasDebugDraw::GetCommandsData)
        .function("GetCommandsSize", &CanvasDebugDraw::GetCommandsSize)
        .function("GetCommandStride", &CanvasDebugDraw::GetCommandStride)
        .function("ClearCommands", &CanvasDebugDraw::ClearCommands)
        .function("GetMaxCommands", &CanvasDebugDraw::GetMaxCommands);
}
