#ifndef CANVASDEBUGDRAW_H
#define CANVASDEBUGDRAW_H

#include <box2d/box2d.h>
#include <emscripten.h>
#include <emscripten/bind.h>

class CanvasDebugDraw {
public:
    b2DebugDraw callbacks{};
    emscripten::val context;

    CanvasDebugDraw(emscripten::val ctx);

    static std::string ColorToHTML(b2HexColor color, float alpha = 1.0f);
};

#endif
