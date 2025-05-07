//
// Created by aidar on 30.04.25.
//

#ifndef GRID_H
#define GRID_H
#include "instance.h"


class grid : public instance {
    float step;
    int currentScaleStep = 5;
    Vector2f currentPivot = {0, 0};

public:
    grid() = delete;

    grid(float step);

    void UpdateGrid(float left, float right, float top, float bottom);
};


#endif //GRID_H
