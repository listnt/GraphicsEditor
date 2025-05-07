#include <cstdio>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <SDL/SDL.h>
#include <emscripten/html5.h>
#define GL_GLEXT_PROTOTYPES
#include <cmath>
#include <unistd.h>
#include <GLES2/gl2ext.h>
#include <GL/gl.h>
#include <cstring>

#include <GLFW/glfw3.h>
#include <chrono>
#include <memory>

#include "src/base/grid.h"
#include "src/base/instance.h"
#include "src/base/PickingTexture.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "src/base/BizierCurve.h"

typedef std::function<void(void)> Closer;

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE initContext() {
    EmscriptenWebGLContextAttributes attrs;
    // attrs.alpha = false;
    attrs.depth = true;
    // attrs.stencil = true;
    // attrs.antialias = true;
    // attrs.premultipliedAlpha = false;
    // attrs.preserveDrawingBuffer = false;
    // attrs.failIfMajorPerformanceCaveat = false;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    // attrs.enableExtensionsByDefault = false;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attrs);
    if (!ctx) {
        printf("Webgl ctx could not be created!\n");
        return -1;
    }

    emscripten_webgl_make_context_current(ctx);

    return ctx;
}

GLuint CompileShader(GLenum type, FILE *shaderFile) {
    fseek(shaderFile, 0, SEEK_END);
    long fsize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET); /* same as rewind(f); */

    char *shaderSrc = (char *) malloc(fsize + 1);
    fread(shaderSrc, fsize, 1, shaderFile);
    fclose(shaderFile);

    shaderSrc[fsize] = '\0';

    // use the string, then ...

    // printf("[DEBUG] compiling shader: %d\nsource:\n%s\n", type, shaderSrc);
    GLint compiled;

    // Create the shader object
    GLuint shader = glCreateShader(type);

    if (shader == 0) {
        free(shaderSrc);
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));

            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteShader(shader);
        free(shaderSrc);
        return 0;
    }

    free(shaderSrc);
    return shader;
}


int linkProgram(const GLuint programObject) {
    GLint linked;

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            const auto infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));

            glGetProgramInfoLog(programObject, infoLen, nullptr, infoLog);
            printf("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return GL_FALSE;
    }

    return GL_TRUE;
}


int initShaders(const char *vs, const char *fs) {
    FILE *vShaderFile = fopen(vs, "rb");
    FILE *fragmentShaderFile = fopen(fs, "rb");

    auto vertexShader = CompileShader(GL_VERTEX_SHADER, vShaderFile);
    auto fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderFile);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    // Create the program object
    const GLuint programObject = glCreateProgram();
    if (programObject == 0)
        return 0;
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    linkProgram(programObject);
    glUseProgram(programObject);


    return programObject;
}

std::shared_ptr<PickingTexture> pickingTexture(new PickingTexture);
std::unordered_map<int, std::shared_ptr<instance> > opaqueObjs;
std::shared_ptr<grid> gridV;
Matrix4x4 projection;
Matrix4x4 camera;
int focusedObj = -1;
bool isCaptured = false;
bool isMouseDown = false;
bool isContextOpen = false;
int mouseClickedX = 0, mouseClickedY = 0;
float left, right, top, bottom, stepx = 1, stepy = 1;;

std::vector<std::pair<const char *, std::function<void()> > > contextMenu = {
    {
        "Add new Curve", []() {
        }
    },
    {
        "Delete Curve", []() {
        }
    }
};


int oldTimestamp = 0;

bool keyboarControl(int k, const EmscriptenKeyboardEvent *e, void *u) {
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

    for (auto it = opaqueObjs.begin(); it != opaqueObjs.end(); ++it) {
        it->second->Resize(top, bottom);
    }

    return true;
}

bool mouseControlDown(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    isMouseDown = true;

    glUseProgram(userData->pickingProgramObject);

    auto z = pickingTexture->GetZIndex(e->targetX, userData->WindowHeight - e->targetY);

    if (e->button == 0) {
        if (opaqueObjs.contains(z)) {
            if (focusedObj != -1 && focusedObj != z) {
                opaqueObjs[focusedObj]->Unpick();
            }

            focusedObj = z;
            opaqueObjs[z]->Pick();

            if (auto b = std::dynamic_pointer_cast<BizierCurve>(opaqueObjs[z])) {
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

                isCaptured = b->PickPoint(p.x, p.y, top, bottom);
            }
        } else {
            if (focusedObj != -1) {
                opaqueObjs[focusedObj]->Unpick();
                focusedObj = -1;
            }
        }
    }

    glUseProgram(userData->programObject);

    mouseClickedX = e->targetX;
    mouseClickedY = userData->WindowHeight - e->targetY;

    return true;
}

bool mouseControlUp(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    if (e->button == 2) {
        isContextOpen = true;
    }

    if (e->button == 0) {
        isMouseDown = false;
    }

    return true;
}

bool mouseControlMove(int eventType, const EmscriptenMouseEvent *e, void *) {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    if (io.WantCaptureMouse) {
        return true;
    }

    if (isCaptured && focusedObj != -1) {
        if (auto b = std::dynamic_pointer_cast<BizierCurve>(opaqueObjs[focusedObj]); b != nullptr && isMouseDown) {
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

            b->MovePoint(p.x, p.y, top, bottom);
        }
    }

    if (isMouseDown && focusedObj == -1) {
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

        for (auto it = opaqueObjs.begin(); it != opaqueObjs.end(); ++it) {
            it->second->Resize(top, bottom);
        }
    }

    return true;
}

bool mouseControlWheel(int eventType, const EmscriptenWheelEvent *e, void *) {
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

    for (auto it = opaqueObjs.begin(); it != opaqueObjs.end(); ++it) {
        it->second->Resize(top, bottom);
    }

    return true;
}

void AddBizierCurve() {
    int freezIndex = 0;
    for (int i = USER_SPACE; i < UI_SPACE; i++) {
        if (!opaqueObjs.contains(i)) {
            freezIndex = i;
        }
    }

    std::shared_ptr<BizierCurve> bizier = std::make_shared<BizierCurve>(4, freezIndex, left, right, top, bottom);

    opaqueObjs[freezIndex] = bizier;
}

void buildGui() {
    if (isContextOpen) {
        isContextOpen = false;
        ImGui::OpenPopup("context");
    }

    if (ImGui::BeginPopup("context")) {
        if (focusedObj != -1) {
            auto color = opaqueObjs[focusedObj]->getColor()[0];
            ImVec4 colorv = ImVec4(color.x, color.y, color.z, color.w);

            bool open_popup = ImGui::ColorButton("MyColor##3b", colorv);
            ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
            open_popup |= ImGui::Button("ChangeColor");
            if (open_popup) {
                ImGui::OpenPopup("mypicker");
            }
            if (ImGui::BeginPopup("mypicker")) {
                ImGui::Text("Choose color");
                ImGui::Separator();
                if (ImGui::ColorPicker4("##picker", (float *) &colorv)) {
                    opaqueObjs[focusedObj]->SetUniformColor({colorv.x, colorv.y, colorv.z, colorv.w});
                };
                ImGui::EndPopup();
            }


            if (ImGui::MenuItem("Delete")) {
                opaqueObjs.erase(focusedObj);
                isCaptured = false;
                focusedObj = -1;
            }
        }
        if (ImGui::MenuItem("Add bizier")) {
            AddBizierCurve();
        }
        ImGui::EndPopup();
    }
}

SDL_Window *window;
SDL_GLContext gl_context;

void initGui() {
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

void initScene() {
    left = -70.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);
    right = 70.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);
    top = 70.0;
    bottom = -70.0;

    stepy = 1.0;
    stepx = 1.0 / (1.0 * userData->WindowHeight / userData->WindowWidth);

    std::shared_ptr<instance> pivot(new instance());

    gridV = std::make_shared<grid>(5);

    std::shared_ptr<BizierCurve> bizier = std::make_shared<BizierCurve>(4,USER_SPACE, left, right, top, bottom);

    opaqueObjs[bizier->getZIndex()] = bizier;

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

///
// Draw a triangle using the shader pair created in Init()
//
int counter = 0;
bool show_demo_window = true;

void Draw() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    pickingTexture->EnableWrite();

    glClearColor(0, 0, 0, 1); // background for picking
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(10);

    glUseProgram(userData->pickingProgramObject);

    for (const auto &obj: opaqueObjs) {
        obj.second->Picking(projection, camera);
    }

    pickingTexture->DisableWrite();

    glUseProgram(userData->programObject);

    glBindTexture(GL_TEXTURE_2D, 0);

    glClearColor(0.2, 0.2, 0.2, 0.2); // background for color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLineWidth(2);

    gridV->Render(projection, camera);

    for (const auto &obj: opaqueObjs) {
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

UserData *userData;

#ifdef __cplusplus

extern "C" {
#endif
int EMSCRIPTEN_KEEPALIVE init(const int width, const int height, bool debug) {
    if (userData) {
        printf("window already initialized\n");

        return GL_TRUE;
    }


    userData = static_cast<UserData *>(malloc(sizeof(UserData)));
    userData->WindowHeight = height;
    userData->WindowWidth = width;


    if (!debug) {
        auto ctx = initContext();
    }

    const auto pickingProgramObject = initShaders("/shaders/picking.vert", "/shaders/picking.frag");
    userData->pickingProgramObject = pickingProgramObject;

    const auto programObject = initShaders("/shaders/.vert", "/shaders/.frag");
    userData->programObject = programObject;

    glUseProgram(userData->programObject);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST); //GL_BLEND | GL_CULL_FACE
    initScene();

    emscripten_set_mousedown_callback("#canvas", nullptr, 0, mouseControlDown);
    emscripten_set_mouseup_callback("#canvas", nullptr, 0, mouseControlUp);
    emscripten_set_mousemove_callback("#canvas", nullptr, 0, mouseControlMove);
    emscripten_set_wheel_callback("#canvas", nullptr, 0, mouseControlWheel);
    emscripten_set_keydown_callback("#canvas", nullptr, 0, keyboarControl);

    emscripten_set_main_loop(Draw, 0, true);

    return GL_TRUE;
}

#ifdef __cplusplus
}
#endif

int main(int argc, char **argv) {
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    window = SDL_CreateWindow("title",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);

    initGui();
    init(640, 480, true);

    return 1;
}
