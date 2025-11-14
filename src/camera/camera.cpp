#include "camera.h"
#include <iostream>

void Camera::setPosition(glm::vec3 pos) {

    m_position = pos;
    m_viewGarbage = true;

}

void Camera::setLook(glm::vec3 look) {

    m_look = look;
    m_viewGarbage = true;

}

void Camera::setUp(glm::vec3 up) {

    m_up = glm::normalize(up);
    m_viewGarbage = true;

}

void Camera::setAspectRatio(float aspectRatio) {

    m_aspectRatio = aspectRatio;
    m_projGarbage = true;

}

void Camera::setAngles(float heightAngle) {

    m_heightAngle = heightAngle;
    m_widthAngle = 2.0f * atan(m_aspectRatio * tan(heightAngle / 2.0f));
    m_projGarbage = true;

}

void Camera::setNearPlane(float nearPlane) {

    m_nearPlane = nearPlane;
    m_projGarbage = true;

}
void Camera::setFarPlane(float farPlane) {

    m_farPlane = farPlane;
    m_projGarbage = true;

}

Camera::Camera() :

    m_position(0, 0, 0),
    m_look(0, 0, 1),
    m_up(0, 1, 0),
    m_heightAngle(0.0f),
    m_aspectRatio(1.0f),
    m_nearPlane(1.0f),
    m_farPlane(1.0f),
    m_viewGarbage(true),
    m_projGarbage(true)

{}


glm::mat4 Camera::getViewMatrix() const {

    /*if (m_viewGarbage) */updateViewMatrix();
    return m_viewMatrix;

}

glm::mat4 Camera::getInverseViewMatrix() const {

    /*if (m_viewGarbage) */updateViewMatrix();
    return m_invViewMatrix;

}

glm::mat4 Camera::getProjectionMatrix() const {

    /*if (m_projGarbage) */updateProjectionMatrix();
    return m_projMatrix;

}

void Camera::updateViewMatrix() const {

    glm::vec3 w = -glm::normalize(m_look);
    glm::vec3 v = glm::normalize(m_up - (glm::dot(m_up, w) * w));
    glm::vec3 u = glm::cross(v, w);

    glm::mat4 rotate(
        u.x, v.x, w.x, 0,
        u.y, v.y, w.y, 0,
        u.z, v.z, w.z, 0,
        0,   0,   0,   1
        );

    glm::mat4 translate(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -m_position.x, -m_position.y, -m_position.z, 1
        );

    m_viewMatrix = rotate * translate;
    m_invViewMatrix = glm::inverse(m_viewMatrix);
    m_viewGarbage = false;

}

void Camera::updateProjectionMatrix() const {

    float scale_x = 1 / (m_farPlane * glm::tan(m_widthAngle / 2.0f));
    float scale_y = 1 / (m_farPlane * glm::tan(m_heightAngle / 2.0f));
    float scale_z = 1 / m_farPlane;

    glm::mat4 s_xyz(
        scale_x, 0, 0, 0,
        0, scale_y, 0, 0,
        0, 0, scale_z, 0,
        0, 0, 0, 1
        );

    float c = -m_nearPlane / m_farPlane;

    glm::mat4 m_pp(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1 / (1 + c), -c / (1 + c),
        0, 0, -1, 0
        );

    glm::mat4 remap(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -2, 0,
        0, 0, 0, 1
        );

    m_projMatrix = remap * m_pp * s_xyz;
    m_projGarbage = false;

}

void Camera::translate(const glm::vec3& delta) {

    m_position += delta;
    m_viewGarbage = true;

}

void Camera::rotate(float deltaX, float deltaY) {

    m_viewGarbage = true;

}
