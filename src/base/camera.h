//
// Created by aidar on 27.04.25.
//

#ifndef CAMERA_H
#define CAMERA_H
#include "base.h"


class camera {
public:
    Matrix4x4 m;
    Vector3f positionOfCamera;
    Vector3f lookAt;
    Vector3f forward;
    Vector3f right;
    Vector3f up;
    camera(Vector3f posOfCamera, Vector3f positionOfTarget) {
        auto positionOfCamera = posOfCamera;

        forward = positionOfCamera - positionOfTarget;
        forward.normalize();

        right = Vector3f({0, 1, 0}).cross(forward);
        right.normalize();

        up = forward.cross(right);
        up.normalize();

        auto tx = -positionOfCamera.dot(right);
        auto ty = -positionOfCamera.dot(up);
        auto tz = -positionOfCamera.dot(forward);

        m.m[0][0] = right.x;
        m.m[1][0] = up.x;
        m.m[2][0] = forward.x;
        m.m[3][0] = 0;
        m.m[0][1] = right.y;
        m.m[1][1] = up.y;
        m.m[2][1] = forward.y;
        m.m[3][1] = 0;
        m.m[0][2] = right.z;
        m.m[1][2] = up.z;
        m.m[2][2] = forward.z;
        m.m[3][2] = 0;
        m.m[0][3] = tx;
        m.m[1][3] = ty;
        m.m[2][3] = tz;
        m.m[3][3] = 1;
    }
};



#endif //CAMERA_H
