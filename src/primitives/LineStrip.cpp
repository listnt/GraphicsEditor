//
// Created by aidar on 08.05.25.
//
#include "LineStrip.h"

bool LineStrip::PickPoint(float x, float y, int top, int bottom) {
    return Line::PickPoint(x, y, top, bottom);
}

void LineStrip::MovePoint(float x, float y, int top, int bottom) {
    Line::MovePoint(x, y, top, bottom);

    points.clear();
    points.push_back(controlPoints[0]);
    for (int i = 1; i < controlPoints.size(); i++) {
        points.push_back(controlPoints[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(), GL_DYNAMIC_DRAW);
}

bool LineStrip::AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom) {
    auto newPickingColor = ZIndexToColor4f(static_cast<int>(std::round(newZIndex)));
    points.push_back(newPoint);
    colors.push_back(newColor);
    pickingColor.push_back(newPickingColor);
    zIndex.push_back(newZIndex);

    if (points.size() == 1) {
        LoadGLBuffers();
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, Color);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector4f), colors.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, ZIndex);
        glBufferData(GL_ARRAY_BUFFER, zIndex.size() * sizeof(float), zIndex.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, PickingColor);
        glBufferData(GL_ARRAY_BUFFER, pickingColor.size() * sizeof(Vector4f), pickingColor.data(), GL_DYNAMIC_DRAW);
    }

    return Line::AddControlPoint(newPoint, newColor, newZIndex, top, bottom);
}

bool LineStrip::isValid() {
    return points.size() >= 2;
}
