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

    if (m_viewGarbage) updateViewMatrix();
    return m_viewMatrix;

}

glm::mat4 Camera::getInverseViewMatrix() const {

    if (m_viewGarbage) updateViewMatrix();
    return m_invViewMatrix;

}

glm::mat4 Camera::getProjectionMatrix() const {

    if (m_projGarbage) updateProjectionMatrix();
    return m_projMatrix;

}

float Camera::getNearPlane() {
    return m_nearPlane;
}

float Camera::getFarPlane() {
    return m_farPlane;
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

    glm::vec4 s_xyzA(scale_x, 0, 0, 0);
    glm::vec4 s_xyzB(0, scale_y, 0, 0);
    glm::vec4 s_xyzC(0, 0, scale_z, 0);
    glm::vec4 s_xyzD(0, 0, 0, 1);

    glm::mat4 s_xyz(
        s_xyzA, s_xyzB, s_xyzC, s_xyzD
    );

    float c = -m_nearPlane / m_farPlane;

    glm::vec4 m_ppA(1, 0, 0, 0);
    glm::vec4 m_ppB(0, 1, 0, 0);
    glm::vec4 m_ppC(0, 0, 1 / (1 + c), -1);
    glm::vec4 m_ppD(0, 0, -c / (1 + c), 0);

    glm::mat4 m_pp(
        m_ppA, m_ppB, m_ppC, m_ppD
    );

    glm::vec4 remapA(1, 0, 0, 0);
    glm::vec4 remapB(0, 1, 0, 0);
    glm::vec4 remapC(0, 0, -2, 0);
    glm::vec4 remapD(0, 0, -1, 1);

    glm::mat4 remap(
        remapA, remapB, remapC, remapD
    );

    m_projMatrix = remap * m_pp * s_xyz;
    m_projGarbage = false;

}

void Camera::translate(const glm::vec3& delta) {

    m_position += delta;
    m_viewGarbage = true;

}

glm::mat3 rotationMatrix(float theta, const glm::vec3& axis) {

    float cosine = cos(theta);
    float sine = sin(theta);

    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;

    glm::mat3 rot;

    rot[0][0] = cosine + ux * ux * (1 - cosine);
    rot[0][1] = ux * uy * (1 - cosine) - uz * sine;
    rot[0][2] = ux * uz * (1 - cosine) + uy * sine;

    rot[1][0] = ux * uy * (1 - cosine) + uz * sine;
    rot[1][1] = cosine + uy * uy * (1 - cosine);
    rot[1][2] = uy * uz * (1 - cosine) - ux * sine;

    rot[2][0] = ux * uz * (1 - cosine) - uy * sine;
    rot[2][1] = uy * uz * (1 - cosine) + ux * sine;
    rot[2][2] = cosine + uz * uz * (1 - cosine);

    return rot;

}

void Camera::rotate(float delta_x, float delta_y) {

    glm::vec3 world(0.0f, 1.0f, 0.0f);
    glm::mat3 rotation_x = rotationMatrix(-delta_x, world);
    m_look = glm::normalize(rotation_x * m_look);

    glm::vec3 right = glm::normalize(glm::cross(m_look, m_up));

    glm::mat3 rotation_y = rotationMatrix(-delta_y, right);
    glm::vec3 look_n = glm::normalize(rotation_y * m_look);

    float dotWithWorldUp = glm::dot(look_n, world);
    if (glm::abs(dotWithWorldUp) < 0.99f) {
        m_look = look_n;
    }

    // Update the up vector to stay perpendicular to look
    updateVectors();
}

void Camera::updateVectors() {

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(m_look, worldUp));
    m_up = glm::normalize(glm::cross(right, m_look));
    updateViewMatrix();

}
