//
// Created by aidar on 08.05.25.
//

#ifndef APP_H
#define APP_H
#include <memory>
#include <SDL/SDL_video.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_opengl3.h"

#include "../base/grid.h"
#include "../base/instance.h"
#include "../base/PickingTexture.h"
#include "../primitives/Line.h"
#include "../primitives/BizierCurve.h"

class App {
protected:
    std::shared_ptr<PickingTexture> pickingTexture = std::make_shared<PickingTexture>();

    std::unordered_map<int, std::shared_ptr<Line> > lines;
    std::shared_ptr<grid> gridV;
    Matrix4x4 projection;
    Matrix4x4 camera;
    int focusedObj = -1;
    bool isCaptured = false;
    bool isMouseDown = false;
    bool isContextOpen = false;
    bool disablePanning = false;
    int mouseClickedX = 0, mouseClickedY = 0;
    float left = -50, right = 50, bottom = -50, top = 50, stepx = 1, stepy = 1;;
    int selectedInstrument = 0;
    const std::vector<char *> instruments = {
        "pointer",
        "line",
        "bizier",
    };

    Vector4f currentColor = {1, 1, 1, 1};

public:
    void PickingPhase();

    void RenderPhase(SDL_Window *window);

    bool keyboarControl(int k, const EmscriptenKeyboardEvent *e, void *u);

    bool mouseControlDown(int eventType, const EmscriptenMouseEvent *e, void *);

    void pickObject(const EmscriptenMouseEvent *e);

    void addLine(const EmscriptenMouseEvent *e);

    void addBizierCurve(const EmscriptenMouseEvent *e);

    bool mouseControlUp(int eventType, const EmscriptenMouseEvent *e, void *);

    bool mouseControlMove(int eventType, const EmscriptenMouseEvent *e, void *);

    bool mouseControlWheel(int eventType, const EmscriptenWheelEvent *e, void *);

    void AddRandomBizierCurve();

    void buildGui();

    void initGui(SDL_Window *window, SDL_GLContext gl_context);

    void initScene();
};


#endif //APP_H
