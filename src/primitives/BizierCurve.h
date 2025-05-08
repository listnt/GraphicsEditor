//
// Created by aidar on 06.05.25.
//

#ifndef BIZIERCURVE_H
#define BIZIERCURVE_H
#include <vector>
#include <GLES2/gl2.h>
#include <random>

#include "Line.h"
#include "../base/base.h"
#include "../base/instance.h"


double computeBinomial(int n, int k);

class BizierCurve : public Line {
protected:
    GLuint ConnectionPointsVAO = 0;
    GLuint ConnectionPointsVBO = 0;
    GLuint ConnectionPointsColor = 0;
    GLuint ConnectionPointsZIndex = 0;

    std::vector<Vector2f> connectionPoints;
    std::vector<Vector4f> connectionColor;
    std::vector<float> connectionZIndex;

    int order;

    std::vector<Vector2f> circlePoints;

    void LoadConnectionPointsBuffer();

public:
    BizierCurve(int order): order(order) {};

    ~BizierCurve();

    BizierCurve(int order, float zOrder, int left, int right, int top, int bottom);

    bool isValid() override;

    void Render(Matrix4x4 projection, Matrix4x4 View) override;

    void MovePoint(float x, float y, int top, int bottom) override;

    bool AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom) override;
};


#endif //BIZIERCURVE_H
