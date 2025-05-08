//
// Created by aidar on 08.05.25.
//

#include "app.h"

#include "../primitives/LineStrip.h"

void App::PickingPhase() {
    pickingTexture->EnableWrite();

    glClearColor(0, 0, 0, 1); // background for picking
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(10);

    glUseProgram(userData->pickingProgramObject);

    for (const auto &obj: lines) {
        obj.second->Picking(projection, camera);
    }

    pickingTexture->DisableWrite();
}

void App::RenderPhase(SDL_Window *window) {
    glUseProgram(userData->programObject);

    glBindTexture(GL_TEXTURE_2D, 0);

    glClearColor(0.2, 0.2, 0.2, 0.2); // background for color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(2);

    gridV->Render(projection, camera);

    for (const auto &obj: lines) {
        obj.second->Render(projection, camera);
    }


    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    buildGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

bool App::keyboarControl(int k, const EmscriptenKeyboardEvent *e, void *u) {
    if (strcmp(e->key, "ArrowLeft") == 0) {
        left -= stepx;
        right -= stepx;

        gridV->UpdateGrid(left, right, top, bottom);
    } else if (strcmp(e->key, "ArrowRight") == 0) {
        right += stepx;
        left += stepx;

        gridV->UpdateGrid(left, right, top, bottom);
    } else if (strcmp(e->key, "ArrowUp") == 0) {
        top += stepy;
        bottom += stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    } else if (strcmp(e->key, "ArrowDown") == 0) {
        top -= stepy;
        bottom -= stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    } else if (strcmp(e->key, "-")) {
        left -= stepx;
        right += stepx;
        top += stepy;
        bottom -= stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    } else if (strcmp(e->key, "+")) {
        left += stepx;
        right -= stepx;
        top -= stepy;
        bottom += stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    } else {
        return true;
    }


    stepy = ((top - bottom) / 100);
    stepx = ((top - bottom) / 100) / (1.0 * userData->WindowHeight / userData->WindowWidth);


    projection = computeOrthoMatrix(left, right, bottom, top, -50000, 2);


    glUseProgram(userData->pickingProgramObject);
    auto ViewModelMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());
    auto projectMat = glGetUniformLocation(userData->pickingProgramObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());

    glUseProgram(userData->programObject);
    projectMat = glGetUniformLocation(userData->programObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());
    ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        it->second->Resize(top, bottom);
    }

    return true;
}

bool App::mouseControlDown(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    glUseProgram(userData->pickingProgramObject);

    if (e->button == 0) {
        isMouseDown = true;
    }

    if (e->button == 0 && selectedInstrument == 0) {
        this->pickObject(e);
    }

    if (e->button == 0 && selectedInstrument == 1) {
        disablePanning = true;
        this->addLine(e);
    }

    if (e->button == 0 && selectedInstrument == 2) {
        disablePanning = true;
        this->addBizierCurve(e);
    }

    glUseProgram(userData->programObject);


    return true;
}

void App::pickObject(const EmscriptenMouseEvent *e) {
    auto z = pickingTexture->GetZIndex(e->targetX, userData->WindowHeight - e->targetY);


    mouseClickedX = e->targetX;
    mouseClickedY = userData->WindowHeight - e->targetY;

    if (lines.contains(z)) {
        if (focusedObj != -1 && focusedObj != z) {
            lines[focusedObj]->Unpick();
        }

        focusedObj = z;
        lines[z]->Pick();

        Matrix4x4 inv;
        inv = projection * camera;
        inv = inverse(inv);
        auto p = Vector4f(2.0f * e->targetX / userData->WindowWidth - 1.0f,
                          1.0f - 2.0 * e->targetY / userData->WindowHeight, 0, 1);
        p = inv * p;

        p.w = 1.0 / p.w;
        p.x *= p.w;
        p.y *= p.w;
        p.z *= p.w;

        printf("%d %d %f %f %f\n", e->targetX, userData->WindowHeight - e->targetY, p.x, p.y, p.z);

        isCaptured = lines[z]->PickPoint(p.x, p.y, top, bottom);
    } else {
        if (focusedObj != -1) {
            lines[focusedObj]->Unpick();
            focusedObj = -1;
        }
    }
}

void App::addLine(const EmscriptenMouseEvent *e) {
    Matrix4x4 inv;
    inv = projection * camera;
    inv = inverse(inv);
    auto p = Vector4f(2.0f * e->targetX / userData->WindowWidth - 1.0f,
                      1.0f - 2.0 * e->targetY / userData->WindowHeight, 0, 1);
    p = inv * p;

    p.w = 1.0 / p.w;
    p.x *= p.w;
    p.y *= p.w;
    p.z *= p.w;

    if (focusedObj == -1) {
        for (int i = USER_SPACE; i < UI_SPACE; i++) {
            if (!lines.contains(i)) {
                focusedObj = i;
            }
        }

        std::shared_ptr<Line> line = std::make_shared<LineStrip>();
        lines[focusedObj] = line;
        lines[focusedObj]->Pick();
    }

    if (focusedObj != -1 &&
        !lines[focusedObj]->AddControlPoint(
            {p.x, p.y},
            currentColor,
            focusedObj,
            top, bottom)) {
        lines[focusedObj]->Unpick();
        if (!lines[focusedObj]->isValid()) {
            lines.erase(focusedObj);
        }

        focusedObj = -1;
    }

    if (focusedObj != -1) {
        isCaptured = lines[focusedObj]->PickPoint(p.x, p.y, top, bottom);
    }
}

void App::addBizierCurve(const EmscriptenMouseEvent *e) {
    Matrix4x4 inv;
    inv = projection * camera;
    inv = inverse(inv);
    auto p = Vector4f(2.0f * e->targetX / userData->WindowWidth - 1.0f,
                      1.0f - 2.0 * e->targetY / userData->WindowHeight, 0, 1);
    p = inv * p;

    p.w = 1.0 / p.w;
    p.x *= p.w;
    p.y *= p.w;
    p.z *= p.w;

    if (focusedObj == -1) {
        for (int i = USER_SPACE; i < UI_SPACE; i++) {
            if (!lines.contains(i)) {
                focusedObj = i;
            }
        }

        std::shared_ptr<BizierCurve> bizier = std::make_shared<BizierCurve>(3);
        lines[focusedObj] = bizier;
        lines[focusedObj]->Pick();
    }

    if (focusedObj != -1 &&
        !lines[focusedObj]->AddControlPoint(
            {p.x, p.y},
            currentColor,
            focusedObj,
            top, bottom)) {
        lines[focusedObj]->Unpick();
        if (!lines[focusedObj]->isValid()) {
            lines.erase(focusedObj);
        }

        focusedObj = -1;
    }

    if (focusedObj != -1) {
        isCaptured = lines[focusedObj]->PickPoint(p.x, p.y, top, bottom);
    }
}


bool App::mouseControlUp(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    if (e->button == 0) {
        disablePanning = false;
        isMouseDown = false;
    }
    if (e->button == 2) {
        isContextOpen = true;
    }


    return true;
}

bool App::mouseControlMove(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    if (isMouseDown && isCaptured && focusedObj != -1) {
        Matrix4x4 inv;
        inv = projection * camera;
        inv = inverse(inv);
        auto p = Vector4f(2.0f * e->targetX / userData->WindowWidth - 1.0f,
                          1.0f - 2.0 * e->targetY / userData->WindowHeight, 0, 1);
        p = inv * p;

        p.w = 1.0 / p.w;
        p.x *= p.w;
        p.y *= p.w;
        p.z *= p.w;

        lines[focusedObj]->MovePoint(p.x, p.y, top, bottom);
    }

    if (isMouseDown && focusedObj == -1 && !disablePanning) {
        // move window
        const float speed = 100.0;
        float moveX = speed * (mouseClickedX - e->targetX) / userData->WindowWidth;
        float moveY = speed * (mouseClickedY - userData->WindowHeight + e->targetY) / userData->WindowHeight;

        mouseClickedX = e->targetX;
        mouseClickedY = userData->WindowHeight - e->targetY;


        left += moveX * stepx;
        right += moveX * stepx;
        top += moveY * stepy;
        bottom += moveY * stepy;

        gridV->UpdateGrid(left, right, top, bottom);

        stepy = ((top - bottom) / 100);
        stepx = ((top - bottom) / 100) / (1.0 * userData->WindowHeight / userData->WindowWidth);

        projection = computeOrthoMatrix(left, right, bottom, top, -50000, 2);


        glUseProgram(userData->pickingProgramObject);
        auto ViewModelMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");
        glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());
        auto projectMat = glGetUniformLocation(userData->pickingProgramObject, "projection");
        glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());

        glUseProgram(userData->programObject);
        projectMat = glGetUniformLocation(userData->programObject, "projection");
        glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());
        ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");
        glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());

        for (auto it = lines.begin(); it != lines.end(); ++it) {
            it->second->Resize(top, bottom);
        }
    }

    return true;
}

bool App::mouseControlWheel(int eventType, const EmscriptenWheelEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    Vector2f mousePos = Vector2f{
        static_cast<float>(e->mouse.targetX),
        static_cast<float>(userData->WindowHeight - e->mouse.targetY)
    };

    const float speed = 4.0;
    float xAspectRatio = mousePos.x / userData->WindowWidth;
    float yAspectRatio = mousePos.y / userData->WindowHeight;

    if (e->deltaY > 0) {
        left += xAspectRatio * speed * stepx;
        right -= (1 - xAspectRatio) * speed * stepx;
        top -= (1 - yAspectRatio) * speed * stepy;
        bottom += yAspectRatio * speed * stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    } else {
        left -= xAspectRatio * speed * stepx;
        right += (1 - xAspectRatio) * speed * stepx;
        top += (1 - yAspectRatio) * speed * stepy;
        bottom -= yAspectRatio * speed * stepy;

        gridV->UpdateGrid(left, right, top, bottom);
    }

    stepy = ((top - bottom) / 100);
    stepx = ((top - bottom) / 100) / (1.0 * userData->WindowHeight / userData->WindowWidth);


    projection = computeOrthoMatrix(left, right, bottom, top, -50000, 2);


    glUseProgram(userData->pickingProgramObject);
    auto ViewModelMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());
    auto projectMat = glGetUniformLocation(userData->pickingProgramObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());

    glUseProgram(userData->programObject);
    projectMat = glGetUniformLocation(userData->programObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());
    ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());

    for (auto it = lines.begin(); it != lines.end(); ++it) {
        it->second->Resize(top, bottom);
    }

    return true;
}

void App::AddRandomBizierCurve() {
    int freezIndex = 0;
    for (int i = USER_SPACE; i < UI_SPACE; i++) {
        if (!lines.contains(i)) {
            freezIndex = i;
        }
    }

    std::shared_ptr<BizierCurve> bizier = std::make_shared<BizierCurve>(3, freezIndex, left, right, top, bottom);

    lines[freezIndex] = bizier;
}

void App::buildGui() {
    if (isContextOpen) {
        isContextOpen = false;
        ImGui::OpenPopup("context");
    }

    if (ImGui::BeginPopup("context")) {
        if (focusedObj != -1) {
            if (ImGui::MenuItem("Delete")) {
                lines.erase(focusedObj);
                isCaptured = false;
                focusedObj = -1;
            }
        }
        if (ImGui::MenuItem("Add bizier")) {
            AddRandomBizierCurve();
        }
        ImGui::EndPopup();
    }

    ImGui::Begin("tools");
    ImVec4 colorv = ImVec4(currentColor.x, currentColor.y, currentColor.z, currentColor.w);
    if (focusedObj != -1) {
        colorv = ImVec4(lines[focusedObj]->getColor()[0].x, lines[focusedObj]->getColor()[0].y,
                        lines[focusedObj]->getColor()[0].z, lines[focusedObj]->getColor()[0].w);
    }

    bool open_popup = ImGui::ColorButton("MyColor##3b", colorv);
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    open_popup |= ImGui::Button("ChangeColor");
    if (open_popup) {
        ImGui::OpenPopup("colorPicker");
    }
    if (ImGui::BeginPopup("colorPicker")) {
        ImGui::Text("Choose color");
        ImGui::Separator();
        if (ImGui::ColorPicker4("##picker", (float *) &colorv)) {
            currentColor = {colorv.x, colorv.y, colorv.z, colorv.w};
            if (focusedObj != -1) {
                lines[focusedObj]->SetUniformColor(currentColor);
            }
        };
        ImGui::EndPopup();
    }

    for (int i = 0; i < instruments.size(); i++) {
        if (ImGui::MenuItem(instruments[i], nullptr, selectedInstrument == i)) {
            selectedInstrument = i;
            if (focusedObj != -1) {
                lines[focusedObj]->Unpick();
                focusedObj = -1;
            }

            isCaptured = false;
        }
    }
    ImGui::End();
}

void App::initGui(SDL_Window *window, SDL_GLContext gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    const char *glsl_version = "#version 300 es";

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void App::initScene() {
    left = -70.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);
    right = 70.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);
    top = 70.0;
    bottom = -70.0;

    stepy = 1.0;
    stepx = 1.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);

    std::shared_ptr<instance> pivot(new instance());

    gridV = std::make_shared<grid>(5);

    std::shared_ptr<BizierCurve> bizier = std::make_shared<BizierCurve>(3,USER_SPACE, left, right, top, bottom);

    lines[bizier->getZIndex()] = bizier;

    projection = computeOrthoMatrix(left, right, bottom, top, -50000, 2);

    glLineWidth(2);

    // glPointSize(10);

    auto projectMat = glGetUniformLocation(userData->programObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());
    auto ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());


    glUseProgram(userData->pickingProgramObject);
    projectMat = glGetUniformLocation(userData->pickingProgramObject, "projection");
    glUniformMatrix4fv(projectMat, 1,GL_FALSE, flatten(projection).data());
    ViewModelMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");
    glUniformMatrix4fv(ViewModelMat, 1,GL_FALSE, flatten(camera).data());

    pickingTexture->Init();

    glUseProgram(userData->programObject);
}
