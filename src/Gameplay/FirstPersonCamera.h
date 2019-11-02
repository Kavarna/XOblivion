#pragma once



#include "Interfaces/ICamera.h"


class FirstPersonCamera : public ICamera
{
    static constexpr const float _rotationSpeed = 150.f;
public:
    FirstPersonCamera(float fov, float aspect, float near, float far);

    // Inherited via ICamera
    virtual void construct() override;
    virtual glm::mat4& getView() override;
    virtual glm::mat4& getProjection() override;


    void setAspectRatio(float fov, float aspect, float near, float far);
    void moveForward(float frametime);
    void moveBackward(float frametime);
    void moveRight(float frametime);
    void moveLeft(float frametime);
    void riseUp(float frametime);

    void rotateLeft(float frametime, float theta);
    void rotateRight(float frametime, float theta);
    void rotateUp(float frametime, float theta);
    void rotateDown(float frametime, float theta);

private:
    glm::mat4               m_view;
    glm::mat4               m_projection;

    glm::vec3               m_forward;
    glm::vec3               m_right;
    glm::vec3               m_up;

    glm::vec3               m_position;

    float                   m_pitch;
    float                   m_yaw;

};
