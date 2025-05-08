//
// Created by aidar on 16.04.25.
//

#ifndef INSTANCE_H
#define INSTANCE_H

#include <algorithm>
#include <iostream>
#include <math.h>
#include <memory>
#include <mutex>
#include <vector>
#include <GLES3/gl3.h>

#include "base.h"


class model {
protected:
    std::vector<Vector2f> points;
    std::vector<float> zIndex;
    std::vector<Vector4f> pickingColor;
    std::vector<Vector4f> colors;

public:
    model(const std::vector<Vector2f> &points, float zIndex, Vector4f color): points(points),
                                                                              zIndex(std::vector<float>(
                                                                                  points.size(), zIndex)),
                                                                              colors(std::vector<Vector4f>(
                                                                                  points.size(), color)),
                                                                              pickingColor(points.size(),
                                                                                  ZIndexToColor4f(
                                                                                      static_cast<int>(std::round(
                                                                                          zIndex)))) {
    }

    model() = default;

    std::vector<Vector2f> getPoints() {
        return points;
    }

    std::vector<Vector4f> getColor() {
        return colors;
    }

    std::vector<float> getZIndeex() {
        return zIndex;
    }

    int getZIndex() {
        return static_cast<int>(std::round(zIndex[0]));
    }

    std::vector<Vector4f> getPickingColor() {
        return pickingColor;
    }
};

class instance : public model {
protected:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint ZIndex = 0;
    GLuint Color = 0;
    GLuint PickingColor = 0;

    GLenum mode = GL_LINE_STRIP;

    GLint ViewModelMat;

    GLint PickingViewMat;

    void LoadGLBuffers();

public:
    Vector3f T, S, R;

    instance(): T(0, 0, 0), S(1, 1, 1), R(0, 0, 0) {
    };

    void loadModel(const std::shared_ptr<model> &obj) {
        if (VAO != 0) {
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &Color);
            glDeleteBuffers(1, &ZIndex);
            glDeleteBuffers(1, &PickingColor);
        }

        points = obj->getPoints();
        zIndex = obj->getZIndeex();
        colors = obj->getColor();
        pickingColor = std::vector<Vector4f>(points.size(), ZIndexToColor4f(static_cast<float>(std::round(zIndex[0]))));

        LoadGLBuffers();
    }


    ~instance() {
        printf("Delete instance\n");
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &ZIndex);
        glDeleteBuffers(1, &Color);
        glDeleteBuffers(1, &PickingColor);
    }

    virtual void Render(Matrix4x4 projection, Matrix4x4 View);

    virtual void Picking(Matrix4x4 projection, Matrix4x4 View);

    void Transform(Vector3f T);

    void Rotate(Vector3f R);

    void Scale(Vector3f S);

    void virtual Pick() {
    };

    void virtual Unpick() {
    };

    virtual void SetUniformColor(Vector4f);
};

#endif //INSTANCE_H
