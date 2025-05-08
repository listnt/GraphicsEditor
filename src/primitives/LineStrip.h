//
// Created by aidar on 08.05.25.
//

#ifndef LINESTRIP_H
#define LINESTRIP_H
#include "Line.h"

class LineStrip : public Line {
public:
    bool PickPoint(float x, float y, int top, int bottom) override;

    void MovePoint(float x, float y, int top, int bottom) override;

    bool AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom) override;

    bool isValid() override;
};

#endif //LINESTRIP_H
