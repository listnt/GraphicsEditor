//
// Created by aidar on 06.05.25.
//

#ifndef BIZIERCURVE_H
#define BIZIERCURVE_H
#include <vector>
#include <GLES2/gl2.h>
#include <random>

#include "base.h"
#include "instance.h"


double computeBinomial(int n, int k);

class BizierCurve : public instance {
    int order;

    GLuint controlPointsVAO;
    GLuint controlPointsVBO;
    GLuint controlPointsColor;
    GLuint controlPointsZIndex;
    GLuint controlPointsPickingColor;

    std::vector<Vector2f> controlPoints;
    std::vector<float> controlZIndex;
    std::vector<Vector4f> controlColors;
    std::vector<Vector4f> controlPickingColors;

    bool isPicked = false;
    int pickedPoint = -1;

    std::vector<Vector2f> circlePoints;

public:
    BizierCurve() {
    };

    ~BizierCurve();

    BizierCurve(int order, float zOrder, int left, int right, int top, int bottom);

    void LoadContolPoinstBuffer(int top, int bottom);

    void Render(Matrix4x4 projection, Matrix4x4 View) override;

    void Picking(Matrix4x4 projection, Matrix4x4 View) override;

    void Resize(int top, int bottom) override;

    void Pick() override;

    void Unpick() override;

    bool PickPoint(float x, float y, int top, int bottom);

    void MovePoint(float x, float y, int top, int bottom);
};


#endif //BIZIERCURVE_H
