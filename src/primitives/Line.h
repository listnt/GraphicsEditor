#ifndef LINE_H
#define LINE_H
#include "../base/instance.h"


class Line : public instance {
protected:
    GLuint controlPointsVAO = 0;
    GLuint controlPointsVBO = 0;
    GLuint controlPointsColor = 0;
    GLuint controlPointsZIndex = 0;
    GLuint controlPointsPickingColor = 0;

    std::vector<Vector2f> controlPoints;
    std::vector<float> controlZIndex;
    std::vector<Vector4f> controlColors;
    std::vector<Vector4f> controlPickingColors;

    bool isPicked = false;
    int pickedPoint = -1;

    std::vector<Vector2f> circlePoints;

    void LoadContolPointstBuffer(Vector4f newColor, Vector4f newPickingColor, int top, int bottom);

public:
    Line() = default;

    ~Line();

    void Pick() override;

    void Unpick() override;

    void Render(Matrix4x4 projection, Matrix4x4 View) override;

    void Picking(Matrix4x4 projection, Matrix4x4 View) override;

    virtual bool isValid();

    virtual bool PickPoint(float x, float y, int top, int bottom);

    virtual void MovePoint(float x, float y, int top, int bottom);

    virtual void Resize(int top, int bottom);

    virtual bool AddControlPoint(Vector2f newPoint, Vector4f newColor, float newZIndex, int top, int bottom);

    void SetUniformColor(Vector4f) override;
};

#endif
