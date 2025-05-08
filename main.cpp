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
#include "src/app/app.h"
#include "src/primitives/BizierCurve.h"

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


///
// Draw a triangle using the shader pair created in Init()
//

SDL_Window *window;
SDL_GLContext gl_context;
UserData *userData;
auto app = new App();

void Draw() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    app->PickingPhase();

    app->RenderPhase(window);
}


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


    app->initGui(window, gl_context);
    app->initScene();

    emscripten_set_mousedown_callback("#canvas", nullptr, 0,
                                      [](int k,const EmscriptenMouseEvent *e,void * u) -> EM_BOOL{
                                      return app->mouseControlDown(k,e,u);
                                      });
    emscripten_set_mouseup_callback("#canvas", nullptr, 0, [](int k,const EmscriptenMouseEvent *e,void * u) -> EM_BOOL{
                                    return app->mouseControlUp(k,e,u);
                                    });
    emscripten_set_mousemove_callback("#canvas", nullptr, 0,
                                      [](int k,const EmscriptenMouseEvent *e,void * u) -> EM_BOOL{
                                      return app->mouseControlMove(k,e,u);
                                      });
    emscripten_set_wheel_callback("#canvas", nullptr, 0, [](int k,const EmscriptenWheelEvent *e,void * u) -> EM_BOOL{
                                  return app->mouseControlWheel(k,e,u);
                                  });
    emscripten_set_keydown_callback("#canvas", nullptr, 0,
                                    [](int k,const EmscriptenKeyboardEvent *e,void * u) -> EM_BOOL{
                                    return app->keyboarControl(k,e,u);
                                    });

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

    init(640, 480, true);

    return 1;
}
