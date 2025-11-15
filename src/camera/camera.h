#pragma once
#include <glm/glm.hpp>

class Camera {

public:

    Camera();

    void setPosition(glm::vec3 pos);
    void setLook(glm::vec3 look);
    void setUp(glm::vec3 up);

    void setAngles(float heightAngle);
    void setAspectRatio(float aspectRatio);

    void setNearPlane(float nearPlane);
    void setFarPlane(float farPlane);


    glm::mat4 getViewMatrix() const;
    glm::mat4 getInverseViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void translate(const glm::vec3& delta);
    void rotate(float delta_x, float delta_y);

private:

    glm::vec3 m_position;
    glm::vec3 m_look;
    glm::vec3 m_up;

    float m_aspectRatio;
    float m_heightAngle;
    float m_widthAngle;

    float m_nearPlane;
    float m_farPlane;

    mutable glm::mat4 m_viewMatrix;
    mutable glm::mat4 m_projMatrix;
    mutable glm::mat4 m_invViewMatrix;

    mutable bool m_viewGarbage;
    mutable bool m_projGarbage;

    void updateViewMatrix() const;
    void updateProjectionMatrix() const;
    void updateVectors();

};
