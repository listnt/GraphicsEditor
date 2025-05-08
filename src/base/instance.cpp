//
// Created by aidar on 16.04.25.
//

#include "instance.h"
#include <numeric>

#include <chrono> // Import the ctime library
#include <GLES2/gl2ext.h>

void instance::LoadGLBuffers() {
    if (VAO != 0) {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &ZIndex);
        glDeleteBuffers(1, &Color);
        glDeleteBuffers(1, &PickingColor);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &ZIndex);
    glGenBuffers(1, &Color);
    glGenBuffers(1, &PickingColor);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(),
                 GL_STATIC_DRAW);

    glUseProgram(userData->programObject);

    auto position = glGetAttribLocation(userData->programObject, "vPosition");
    glVertexAttribPointer(position, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(position);

    glUseProgram(userData->pickingProgramObject);

    auto pickingPosition = glGetAttribLocation(userData->pickingProgramObject, "vPosition");
    glVertexAttribPointer(pickingPosition, 2, GL_FLOAT, 0, sizeof(Vector2f), 0);
    glEnableVertexAttribArray(pickingPosition);

    glUseProgram(userData->programObject);

    glBindBuffer(GL_ARRAY_BUFFER, ZIndex);
    glBufferData(GL_ARRAY_BUFFER, zIndex.size() * sizeof(float), zIndex.data(),
                 GL_STATIC_DRAW);

    auto aZIndex = glGetAttribLocation(userData->programObject, "zIndex");
    glVertexAttribPointer(aZIndex, 1, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aZIndex);

    glUseProgram(userData->pickingProgramObject);

    auto aPickingZIndex = glGetAttribLocation(userData->pickingProgramObject, "zIndex");
    glVertexAttribPointer(aPickingZIndex, 1, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aPickingZIndex);

    glUseProgram(userData->programObject);

    glBindBuffer(GL_ARRAY_BUFFER, Color);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector4f), colors.data(),
                 GL_STATIC_DRAW);

    auto aColor = glGetAttribLocation(userData->programObject, "vColor");
    glVertexAttribPointer(aColor, 4, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aColor);

    ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");


    glUseProgram(userData->pickingProgramObject);

    glBindBuffer(GL_ARRAY_BUFFER, PickingColor);
    glBufferData(GL_ARRAY_BUFFER, pickingColor.size() * sizeof(Vector4f), pickingColor.data(),
                 GL_STATIC_DRAW);

    auto aPickingColor = glGetAttribLocation(userData->pickingProgramObject, "aColor");
    glVertexAttribPointer(aPickingColor, 4, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aPickingColor);

    PickingViewMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");

    glBindVertexArray(0);

    glUseProgram(userData->programObject);
}

void instance::Render(Matrix4x4 projection, Matrix4x4 View) {
    glBindVertexArray(this->VAO);
    auto ViewModel = View * translate(T.x, T.y, T.z) *
                     roll(R.z) * pitch(R.y) * yaw(R.x) *
                     scale(S.x, S.y, S.z);

    glUniformMatrix4fv(ViewModelMat, 1, GL_FALSE, flatten(ViewModel).data());

    if (points.size() < 2) {
        glBindVertexArray(0);
        return;
    }

    glDrawArrays(this->mode, 0, points.size());
    glBindVertexArray(0);
}

void instance::Picking(Matrix4x4 projection, Matrix4x4 View) {
    glBindVertexArray(this->VAO);
    auto ViewModel = View * translate(T.x, T.y, T.z) *
                     roll(R.z) * pitch(R.y) * yaw(R.x)
                     * scale(S.x, S.y, S.z);

    glUniformMatrix4fv(PickingViewMat, 1, GL_FALSE, flatten(ViewModel).data());

    glDrawArrays(this->mode, 0, points.size());
    glBindVertexArray(0);
}

void instance::Transform(Vector3f T) { this->T = T; }

void instance::Rotate(Vector3f R) { this->R = R; }

void instance::Scale(Vector3f S) { this->S = S; }

void instance::SetUniformColor(Vector4f newColor) {
    for (auto &color: colors) {
        color = newColor;
    }

    glBindBuffer(GL_ARRAY_BUFFER, Color);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector4f), colors.data(),
                 GL_STATIC_DRAW);
}
