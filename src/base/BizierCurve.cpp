#include "BizierCurve.h"

BizierCurve::~BizierCurve() {
    printf("deleted bizier\n");
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &ZIndex);
    glDeleteBuffers(1, &Color);
    glDeleteBuffers(1, &PickingColor);
    glDeleteBuffers(1, &controlPointsVAO);
    glDeleteBuffers(1, &controlPointsVBO);
    glDeleteBuffers(1, &controlPointsColor);
    glDeleteBuffers(1, &controlPointsZIndex);
    glDeleteBuffers(1, &controlPointsPickingColor);
}

BizierCurve::BizierCurve(int order, float zOrder, int left, int right, int top, int bottom): order(order) {
    if (order < 2) {
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, right - left);
    std::uniform_int_distribution<> distrib1(0, top - bottom);
    std::uniform_int_distribution<> distrib2(0, 255);

    for (int i = 0; i <= order; i++) {
        controlPoints.push_back(Vector2f(distrib(gen) + left, distrib1(gen) + bottom));
    }


    for (double t = 0.0; t <= 1.0; t += 0.01) {
        double bCurveXt = 0;
        double bCurveYt = 0;

        for (int i = 0; i <= order; ++i) {
            double binomialCoeff = computeBinomial(order, i);
            double basis = binomialCoeff * std::pow(1 - t, order - i) * std::pow(t, i);
            bCurveXt += basis * controlPoints[i].x;
            bCurveYt += basis * controlPoints[i].y;
        }

        points.push_back(Vector2f(bCurveXt, bCurveYt));
    }

    colors = std::vector<Vector4f>(points.size(), {
                                       static_cast<GLfloat>(distrib2(gen) / 255.0),
                                       static_cast<GLfloat>(distrib2(gen) / 255.0),
                                       static_cast<GLfloat>(distrib2(gen) / 255.0),
                                       1.0
                                   });
    zIndex = std::vector<float>(points.size(), zOrder);
    pickingColor = std::vector<Vector4f>(points.size(), ZIndexToColor4f(static_cast<float>(zOrder)));

    LoadGLBuffers();
    LoadContolPoinstBuffer(top, bottom);
}

void BizierCurve::LoadContolPoinstBuffer(int top, int bottom) {
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

        auto circleColors = std::vector<Vector4f>(circle.size(), colors[0]);

        auto zIndexes = std::vector<float>(circle.size(), UI_SPACE);

        auto circlePickingColors = std::vector<Vector4f>(circle.size(), pickingColor[0]);


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

void BizierCurve::Render(Matrix4x4 projection, Matrix4x4 View) {
    instance::Render(projection, View);
    if (isPicked) {
        glBindVertexArray(controlPointsVAO);
        glDrawArrays(GL_TRIANGLES, 0, circlePoints.size());
    }
}

void BizierCurve::Picking(Matrix4x4 projection, Matrix4x4 View) {
    instance::Picking(projection, View);
    if (isPicked) {
        glBindVertexArray(controlPointsVAO);
        glDrawArrays(GL_TRIANGLES, 0, circlePoints.size());
    }
}

void BizierCurve::Resize(int top, int bottom) {
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

void BizierCurve::Pick() {
    isPicked = true;
}

void BizierCurve::Unpick() {
    isPicked = false;
    pickedPoint = -1;
}

bool BizierCurve::PickPoint(float x, float y, int top, int bottom) {
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

    printf("current point %d\n", currentPickedPoint);

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

void BizierCurve::MovePoint(float x, float y, int top, int bottom) {
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

    points.clear();

    for (double t = 0.0; t <= 1.0; t += 0.01) {
        double bCurveXt = 0;
        double bCurveYt = 0;

        for (int i = 0; i <= order; ++i) {
            double binomialCoeff = computeBinomial(order, i);
            double basis = binomialCoeff * std::pow(1 - t, order - i) * std::pow(t, i);

            bCurveXt += basis * controlPoints[i].x;
            bCurveYt += basis * controlPoints[i].y;
        }

        points.push_back(Vector2f(bCurveXt, bCurveYt));
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(), GL_DYNAMIC_DRAW);
}


// Compute binomial coefficient efficiently
double computeBinomial(int n, int k) {
    if (k < 0 || k > n) return 0.0f;
    if (k == 0 || k == n) return 1.0f;

    // Use multiplicative formula to avoid large intermediate factorials
    k = std::min(k, n - k);
    float result = 1.0f;
    for (int i = 0; i < k; ++i) {
        result *= static_cast<float>(n - i) / static_cast<float>(i + 1);
    }
    return result;
}
