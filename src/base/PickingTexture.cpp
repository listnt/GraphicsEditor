//
// Created by aidar on 04.05.25.
//

#include "PickingTexture.h"


int PickingTexture::GetZIndex(int x, int y) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, PickingObjectBuffer);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    float data[4];
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, data);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    printf("data %f %f %f %f\n", data[0], data[1], data[2], data[3]);

    return data[0] * 255 +
           data[1] * 255 * 256 +
           data[2] * 255 * 256 * 256;;
}

void PickingTexture::EnableWrite() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, PickingObjectBuffer);
    // glBindTexture(GL_TEXTURE_2D, PickingObjectTexture);
}

void PickingTexture::DisableWrite() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glBindTexture(GL_TEXTURE_2D, 0);
}

bool PickingTexture::Init() {
    glGenFramebuffers(1, &PickingObjectBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, PickingObjectBuffer);

    // printf("init picking %d\n", PickingObjectBuffer);

    glGenTextures(1, &PickingObjectTexture);
    glBindTexture(GL_TEXTURE_2D, PickingObjectTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, userData->WindowWidth, userData->WindowHeight,
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           PickingObjectTexture, 0);

    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8, userData->WindowWidth, userData->WindowHeight);
    //
    // // Create the texture object for the depth bufferu0
    // glGenTextures(1, &PickingDepthTexture);
    // glBindTexture(GL_TEXTURE_2D, PickingDepthTexture);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PickingObjectTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // Verify that the FBO is correct
    GLenum Fstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Fstatus != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Fstatus);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glBindRenderbuffer(GL_RENDERBUFFER, 0);


    return true;
}
