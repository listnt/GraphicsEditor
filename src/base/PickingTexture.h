//
// Created by aidar on 04.05.25.
//

#ifndef PICKINGTEXTURE_H
#define PICKINGTEXTURE_H
#include "base.h"


class PickingTexture {
protected:
    GLuint PickingObjectBuffer;
    GLuint PickingObjectTexture;
    GLuint PickingDepthTexture;
    GLuint RBO;

public:
    PickingTexture() {
    };

    bool Init();

    int GetZIndex(int x, int y);

    void EnableWrite();

    void DisableWrite();
};


#endif //PICKINGTEXTURE_H
