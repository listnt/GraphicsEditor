#include "Line.h"

#include <random>

Line::~Line() {
    printf("delete line\n");
    glDeleteVertexArrays(1, &controlPointsVAO);
    glDeleteBuffers(1, &controlPointsVBO);
    glDeleteBuffers(1, &controlPointsColor);
    glDeleteBuffers(1, &controlPointsZIndex);
    glDeleteBuffers(1, &controlPointsPickingColor);
}

void Line::Pick() {
    isPicked = true;
}

void Line::Unpick() {
    isPicked = false;
    pickedPoint = -1;
}

void Line::Render(Matrix4x4 projection, Matrix4x4 View) {
    instance::Render(projection, View);
    if (isPicked && !controlPoints.empty()) {
        glBindVertexArray(controlPointsVAO);
        glDrawArrays(GL_TRIANGLES, 0, circlePoints.size());
    }
}

void Line::Picking(Matrix4x4 projection, Matrix4x4 View) {
    instance::Picking(projection, View);
    if (isPicked) {
        glBindVertexArray(controlPointsVAO);
        glDrawArrays(GL_TRIANGLES, 0, circlePoints.size());
    }
}

bool Line::isValid() {
    return false;
}

void Line::LoadContolPointstBuffer(Vector4f newColor, Vector4f newPickingColor, int top, int bottom) {
    if (controlPointsVAO != 0) {
        glDeleteBuffers(1, &controlPointsVBO);
        glDeleteBuffers(1, &controlPointsColor);
        glDeleteBuffers(1, &controlPointsZIndex);
        glDeleteBuffers(1, &controlPointsPickingColor);
    }

    glGenVertexArrays(1, &controlPointsVAO);
    glGenBuffers(1, &controlPointsVBO);
    glGenBuffers(1, &controlPointsZIndex);
    glGenBuffers(1, &controlPointsColor);
    glGenBuffers(1, &controlPointsPickingColor);

    glBindVertexArray(controlPointsVAO);

    circlePoints.clear();
    controlZIndex = std::vector<float>();
    controlColors = std::vector<Vector4f>();
    controlPickingColors = std::vector<Vector4f>();
    circlePoints = std::vector<Vector2f>();

    for (int i = 0; i < controlPoints.size(); i++) {
        auto circle = CreateCircle(controlPoints[i], (top - bottom) / (100.0 / 1.5), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), newColor);

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), newPickingColor);


        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());
    }

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(Vector2f), circlePoints.data(),
                 GL_DYNAMIC_DRAW);

    glUseProgram(userData->programObject);

    auto position = glGetAttribLocation(userData->programObject, "vPosition");
    glVertexAttribPointer(position, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(position);

    glUseProgram(userData->pickingProgramObject);

    auto pickingPosition = glGetAttribLocation(userData->pickingProgramObject, "vPosition");
    glVertexAttribPointer(pickingPosition, 2, GL_FLOAT, 0, sizeof(Vector2f), 0);
    glEnableVertexAttribArray(pickingPosition);

    glUseProgram(userData->programObject);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsZIndex);
    glBufferData(GL_ARRAY_BUFFER, controlZIndex.size() * sizeof(float), controlZIndex.data(),
                 GL_STATIC_DRAW);

    auto aZIndex = glGetAttribLocation(userData->programObject, "zIndex");
    glVertexAttribPointer(aZIndex, 1, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aZIndex);

    glUseProgram(userData->pickingProgramObject);

    auto aPickingZIndex = glGetAttribLocation(userData->pickingProgramObject, "zIndex");
    glVertexAttribPointer(aPickingZIndex, 1, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aPickingZIndex);

    glUseProgram(userData->programObject);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
    glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(),
                 GL_STATIC_DRAW);

    auto aColor = glGetAttribLocation(userData->programObject, "vColor");
    glVertexAttribPointer(aColor, 4, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aColor);

    ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");


    glUseProgram(userData->pickingProgramObject);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsPickingColor);
    glBufferData(GL_ARRAY_BUFFER, controlPickingColors.size() * sizeof(Vector4f), controlPickingColors.data(),
                 GL_STATIC_DRAW);

    auto aPickingColor = glGetAttribLocation(userData->pickingProgramObject, "aColor");
    glVertexAttribPointer(aPickingColor, 4, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aPickingColor);

    PickingViewMat = glGetUniformLocation(userData->pickingProgramObject, "viewModel");

    glBindVertexArray(0);

    glUseProgram(userData->programObject);
}

bool Line::PickPoint(float x, float y, int top, int bottom) {
    int currentPickedPoint = -1;
    auto r = abs(top - bottom) / (100.0 / 1.5);
    for (int i = 0; i < controlPoints.size(); i++) {
        if (x < controlPoints[i].x + r &&
            x > controlPoints[i].x - r &&
            y < controlPoints[i].y + r &&
            y > controlPoints[i].y - r) {
            currentPickedPoint = i;
        }
    }

    if (currentPickedPoint >= 0 && currentPickedPoint != pickedPoint) {
        pickedPoint = currentPickedPoint;

        glBindVertexArray(controlPointsVAO);

        circlePoints.clear();
        controlColors.clear();
        controlZIndex.clear();
        controlPickingColors.clear();

        for (int i = 0; i < controlPoints.size(); i++) {
            auto circle = CreateCircle(controlPoints[i], abs(top - bottom) / (100.0 / 1.5), 30);

            auto circleColors = std::vector<Vector4f>(circle.size(), colors[0]);

            auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

            auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

            circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
            controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
            controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
            controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(),
                                        circlePickingColors.end());
        }

        auto circle = CreateCircle(controlPoints[pickedPoint], abs(top - bottom) / (100.0 / 2.0), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), {1, 1, 1, 1});

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE - 1);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());


        glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
        glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(Vector2f), circlePoints.data(),GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
        glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsZIndex);
        glBufferData(GL_ARRAY_BUFFER, controlZIndex.size() * sizeof(float), controlZIndex.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsPickingColor);
        glBufferData(GL_ARRAY_BUFFER, controlPickingColors.size() * sizeof(Vector4f), controlPickingColors.data(),
                     GL_DYNAMIC_DRAW);

        return true;
    }

    return currentPickedPoint != -1;
}

void Line::MovePoint(float x, float y, int top, int bottom) {
    if (pickedPoint == -1) {
        return;
    }

    controlPoints[pickedPoint].x = x;
    controlPoints[pickedPoint].y = y;

    glBindVertexArray(controlPointsVAO);

    circlePoints.clear();
    controlColors.clear();
    controlZIndex.clear();
    controlPickingColors.clear();

    for (int i = 0; i < controlPoints.size(); i++) {
        auto circle = CreateCircle(controlPoints[i], abs(top - bottom) / (100.0 / 1.5), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), colors[0]);

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(),
                                    circlePickingColors.end());
    }

    auto circle = CreateCircle(controlPoints[pickedPoint], abs(top - bottom) / (100.0 / 2.0), 30);

    auto circleColors = std::vector<Vector4f>(circle.size(), {1, 1, 1, 1});

    auto zIndexes = std::vector<float>(circle.size(), UI_SPACE - 1);

    auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

    circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
    controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
    controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
    controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());


    glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(Vector2f), circlePoints.data(),GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
    glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsZIndex);
    glBufferData(GL_ARRAY_BUFFER, controlZIndex.size() * sizeof(float), controlZIndex.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsPickingColor);
    glBufferData(GL_ARRAY_BUFFER, controlPickingColors.size() * sizeof(Vector4f), controlPickingColors.data(),
                 GL_DYNAMIC_DRAW);
}


void Line::Resize(int top, int bottom) {
    glBindVertexArray(controlPointsVAO);

    circlePoints.clear();
    controlColors.clear();
    controlZIndex.clear();
    controlPickingColors.clear();

    for (int i = 0; i < controlPoints.size(); i++) {
        auto circle = CreateCircle(controlPoints[i], abs(top - bottom) / (100.0 / 1.5), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), colors[0]);

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());
    }

    if (pickedPoint >= 0) {
        auto circle = CreateCircle(controlPoints[pickedPoint], abs(top - bottom) / (100.0 / 2.0), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), {1, 1, 1, 1});

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE - 1);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);

        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());
    }

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(Vector2f), circlePoints.data(),GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
    glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsZIndex);
    glBufferData(GL_ARRAY_BUFFER, controlZIndex.size() * sizeof(float), controlZIndex.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsPickingColor);
    glBufferData(GL_ARRAY_BUFFER, controlPickingColors.size() * sizeof(Vector4f), controlPickingColors.data(),
                 GL_DYNAMIC_DRAW);
}

void Line::SetUniformColor(Vector4f vector4_f) {
    instance::SetUniformColor(vector4_f);

    for (auto &color: controlColors) {
        color = vector4_f;
    }

    glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
    glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(), GL_DYNAMIC_DRAW);
}

bool Line::AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom) {
    auto newPickingColor = ZIndexToColor4f(static_cast<int>(std::round(newZIndex)));

    controlPoints.push_back(newPoint);

    if (controlPoints.size() == 1) {
        LoadContolPointstBuffer(newColor, newPickingColor, top, bottom);
    } else {
        auto circle = CreateCircle(newPoint, abs(top - bottom) / (100.0 / 1.5), 30);

        auto circleColors = std::vector<Vector4f>(circle.size(), newColor);

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), newPickingColor);

        circlePoints.insert(circlePoints.end(), circle.begin(), circle.end());
        controlColors.insert(controlColors.end(), circleColors.begin(), circleColors.end());
        controlZIndex.insert(controlZIndex.end(), zIndexes.begin(), zIndexes.end());
        controlPickingColors.insert(controlPickingColors.end(), circlePickingColors.begin(), circlePickingColors.end());

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsVBO);
        glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(Vector2f), circlePoints.data(),GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsColor);
        glBufferData(GL_ARRAY_BUFFER, controlColors.size() * sizeof(Vector4f), controlColors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsZIndex);
        glBufferData(GL_ARRAY_BUFFER, controlZIndex.size() * sizeof(float), controlZIndex.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, controlPointsPickingColor);
        glBufferData(GL_ARRAY_BUFFER, controlPickingColors.size() * sizeof(Vector4f), controlPickingColors.data(),
                     GL_DYNAMIC_DRAW);
    }

    return true;
}
