#include "FirstPersonCamera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>

glm::vec4 g_forward = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
glm::vec4 g_right   = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
glm::vec4 g_up      = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

FirstPersonCamera::FirstPersonCamera(float fov, float aspect, float near, float far) :
    m_forward(g_forward), m_right(g_right), m_up(g_up),
    m_position(glm::vec3(0.0f, 0.0f, 5.0f)),
    m_pitch(0.0f), m_yaw(0.0f)
{
    setAspectRatio(fov, aspect, near, far);
}

void FirstPersonCamera::construct()
{
    glm::mat4x4 rotationMatrix = glm::eulerAngleXY(m_pitch, m_yaw);
    m_forward = g_forward * rotationMatrix;
    m_right = g_right * rotationMatrix;
    m_up = glm::cross(m_right, m_forward);
    m_view = glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4& FirstPersonCamera::getView()
{
    return m_view;
}

glm::mat4& FirstPersonCamera::getProjection()
{
    return m_projection;
}

void FirstPersonCamera::setAspectRatio(float fov, float aspect, float near, float far)
{
    m_projection = glm::perspective(fov, aspect, near, far);
    m_projection[1][1] *= -1;
}

void FirstPersonCamera::moveForward(float frametime)
{
    m_position += m_forward * frametime;
}

void FirstPersonCamera::moveBackward(float frametime)
{
    m_position -= m_forward * frametime;
}

void FirstPersonCamera::moveRight(float frametime)
{
    m_position += m_right * frametime;
}

void FirstPersonCamera::moveLeft(float frametime)
{
    m_position -= m_right * frametime;
}

void FirstPersonCamera::riseUp(float frametime)
{
    m_position += m_up * frametime;
}

void FirstPersonCamera::rotateLeft(float frametime, float theta)
{
    rotateRight(frametime, -theta);
}

void FirstPersonCamera::rotateRight(float frametime, float theta)
{
    /*m_forward = glm::rotateY(m_forward, theta);
    m_right = glm::rotateY(m_right, theta);
    m_up = glm::cross(m_right, m_forward);*/
    m_yaw += frametime * theta * _rotationSpeed;
}

void FirstPersonCamera::rotateUp(float frametime, float theta)
{
    m_pitch += frametime * theta * _rotationSpeed;
}

void FirstPersonCamera::rotateDown(float frametime, float theta)
{
    rotateUp(frametime, -theta);
}

