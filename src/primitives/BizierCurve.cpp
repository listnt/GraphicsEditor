#include "BizierCurve.h"

void BizierCurve::LoadConnectionPointsBuffer() {
    if (ConnectionPointsVAO != 0) {
        glDeleteBuffers(1, &ConnectionPointsVBO);
        glDeleteBuffers(1, &ConnectionPointsColor);
        glDeleteBuffers(1, &ConnectionPointsZIndex);
    }

    glGenVertexArrays(1, &ConnectionPointsVAO);
    glGenBuffers(1, &ConnectionPointsVBO);
    glGenBuffers(1, &ConnectionPointsColor);
    glGenBuffers(1, &ConnectionPointsZIndex);

    glBindVertexArray(ConnectionPointsVAO);

    glUseProgram(userData->programObject);

    glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, connectionPoints.size() * sizeof(Vector2f), connectionPoints.data(),
                 GL_DYNAMIC_DRAW);


    auto position = glGetAttribLocation(userData->programObject, "vPosition");
    glVertexAttribPointer(position, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(position);


    glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsZIndex);
    glBufferData(GL_ARRAY_BUFFER, connectionZIndex.size() * sizeof(float), connectionZIndex.data(),
                 GL_STATIC_DRAW);

    auto aZIndex = glGetAttribLocation(userData->programObject, "zIndex");
    glVertexAttribPointer(aZIndex, 1, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aZIndex);


    glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsColor);
    glBufferData(GL_ARRAY_BUFFER, connectionColor.size() * sizeof(Vector4f), connectionColor.data(),
                 GL_STATIC_DRAW);

    auto aColor = glGetAttribLocation(userData->programObject, "vColor");
    glVertexAttribPointer(aColor, 4, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(aColor);

    ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");

    glBindVertexArray(0);

    glUseProgram(userData->programObject);
}

BizierCurve::~BizierCurve() {
    printf("deleted bizier\n");
    glDeleteVertexArrays(1, &ConnectionPointsVAO);
    glDeleteBuffers(1, &ConnectionPointsVBO);
    glDeleteBuffers(1, &ConnectionPointsColor);
    glDeleteBuffers(1, &ConnectionPointsZIndex);
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

    connectionPoints.push_back(controlPoints[0]);
    for (int i = 1; i < controlPoints.size(); i++) {
        connectionPoints.push_back(controlPoints[i]);
    }

    connectionColor = std::vector<Vector4f>(connectionPoints.size(),
                                            Vector4f{0.4, 0.4, 0.4, 0.4});
    connectionZIndex = std::vector<float>(connectionPoints.size(), zIndex[0]);

    LoadGLBuffers();
    LoadContolPointstBuffer(colors[0], pickingColor[0], top, bottom);
    LoadConnectionPointsBuffer();
}

bool BizierCurve::isValid() {
    return controlPoints.size() >= order + 1;
}

void BizierCurve::Render(Matrix4x4 projection, Matrix4x4 View) {
    Line::Render(projection, View);

    if (isPicked && connectionPoints.size() > 1) {
        glBindVertexArray(ConnectionPointsVAO);
        glDrawArrays(GL_LINE_STRIP, 0, connectionPoints.size());
    }
}

void BizierCurve::MovePoint(float x, float y, int top, int bottom) {
    Line::MovePoint(x, y, top, bottom);

    if (controlPoints.size() == order + 1) {
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
    }


    connectionPoints.clear();

    connectionPoints.push_back(controlPoints[0]);
    for (int i = 1; i < controlPoints.size(); i++) {
        connectionPoints.push_back(controlPoints[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsVBO);
    glBufferData(GL_ARRAY_BUFFER, connectionPoints.size() * sizeof(Vector2f), connectionPoints.data(),
                 GL_DYNAMIC_DRAW);
}

bool BizierCurve::AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom) {
    if (controlPoints.size() == order + 1) {
        return false;
    }

    if (!Line::AddControlPoint(newPoint, newColor, newZIndex, top, bottom)) {
        return false;
    }

    auto newPickingColor = ZIndexToColor4f(static_cast<int>(std::round(newZIndex)));

    if (controlPoints.size() == order + 1) {
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

        colors.clear();
        zIndex.clear();
        pickingColor.clear();

        colors = std::vector<Vector4f>(points.size(), newColor);
        zIndex = std::vector<float>(points.size(), newZIndex);
        pickingColor = std::vector<Vector4f>(points.size(), newPickingColor);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, Color);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector4f), colors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, ZIndex);
        glBufferData(GL_ARRAY_BUFFER, zIndex.size() * sizeof(float), zIndex.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, PickingColor);
        glBufferData(GL_ARRAY_BUFFER, pickingColor.size() * sizeof(Vector4f), pickingColor.data(), GL_DYNAMIC_DRAW);
    }

    if (controlPoints.size() == 1) {
        points.push_back(connectionPoints[0]);

        colors.clear();
        zIndex.clear();
        pickingColor.clear();

        colors = std::vector<Vector4f>(points.size(), newColor);
        zIndex = std::vector<float>(points.size(), newZIndex);
        pickingColor = std::vector<Vector4f>(points.size(), newPickingColor);

        LoadGLBuffers();
    }

    connectionPoints.push_back(newPoint);
    connectionColor.emplace_back(0.4, 0.4, 0.4, 0.4);
    connectionZIndex.push_back(newZIndex);

    if (controlPoints.size() == 1) {
        LoadConnectionPointsBuffer();
    } else if (controlPoints.size() > 1) {
        glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsVBO);
        glBufferData(GL_ARRAY_BUFFER, connectionPoints.size() * sizeof(Vector2f), connectionPoints.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsColor);
        glBufferData(GL_ARRAY_BUFFER, connectionColor.size() * sizeof(Vector4f), connectionColor.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, ConnectionPointsZIndex);
        glBufferData(GL_ARRAY_BUFFER, connectionZIndex.size() * sizeof(float), connectionZIndex.data(),
                     GL_DYNAMIC_DRAW);
    }

    printf("Points: %d, controlPoints, %d, connectionPoints, %d\n", points.size(), controlPoints.size(),
           connectionPoints.size());

    return true;
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
