#include "cylinder.h"
#include <glm/glm.hpp>

#include "cylinder.h"
#include <glm/glm.hpp>


static void insert(std::vector<float>& data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}


static glm::vec3 calculateBodyNormal(const glm::vec3& point) {

    return glm::normalize(glm::vec3(point.x, 0.0f, point.z));
}


static void makeBodyTile(std::vector<float>& data,
                        glm::vec3 topLeft, glm::vec3 topRight,
                        glm::vec3 bottomLeft, glm::vec3 bottomRight) {

    glm::vec3 normalTL = calculateBodyNormal(topLeft);
    glm::vec3 normalTR = calculateBodyNormal(topRight);
    glm::vec3 normalBL = calculateBodyNormal(bottomLeft);
    glm::vec3 normalBR = calculateBodyNormal(bottomRight);

    insert(data, bottomLeft);
    insert(data, normalBL);

    insert(data, topLeft);
    insert(data, normalTL);

    insert(data, topRight);
    insert(data, normalTR);

    insert(data, bottomLeft);
    insert(data, normalBL);

    insert(data, topRight);
    insert(data, normalTR);

    insert(data, bottomRight);
    insert(data, normalBR);
}


static void makeTopCapTile(std::vector<float>& data,
                          glm::vec3 innerStart, glm::vec3 innerEnd,
                          glm::vec3 outerStart, glm::vec3 outerEnd) {

    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    // Triangle 1: innerStart -> innerEnd -> outerEnd
    insert(data, innerStart);
    insert(data, normal);

    insert(data, innerEnd);
    insert(data, normal);

    insert(data, outerEnd);
    insert(data, normal);

    // Triangle 2: innerStart -> outerEnd -> outerStart
    insert(data, innerStart);
    insert(data, normal);

    insert(data, outerEnd);
    insert(data, normal);

    insert(data, outerStart);
    insert(data, normal);
}

static void makeBottomCapTile(std::vector<float>& data,
                             glm::vec3 innerStart, glm::vec3 innerEnd,
                             glm::vec3 outerStart, glm::vec3 outerEnd) {

    glm::vec3 normal(0.0f, -1.0f, 0.0f);

    // Triangle 1: innerStart -> outerEnd -> innerEnd
    insert(data, innerStart);
    insert(data, normal);

    insert(data, outerEnd);
    insert(data, normal);

    insert(data, innerEnd);
    insert(data, normal);

    // Triangle 2: innerStart -> outerStart -> outerEnd
    insert(data, innerStart);
    insert(data, normal);

    insert(data, outerStart);
    insert(data, normal);

    insert(data, outerEnd);
    insert(data, normal);
}


static void makeBodyWedge(std::vector<float>& data,
                         int param1,
                         float currentTheta, float nextTheta) {

    const float radius = 0.5f;
    const float height = 1.0f;
    const float halfHeight = 0.5f;


    for (int i = 0; i < param1; i++) {

        float yBottom = -halfHeight + (float)i / param1 * height;
        float yTop = -halfHeight + (float)(i + 1) / param1 * height;

        glm::vec3 topLeft(
            radius * glm::cos(currentTheta),
            yTop,
            radius * glm::sin(currentTheta)
        );

        glm::vec3 topRight(
            radius * glm::cos(nextTheta),
            yTop,
            radius * glm::sin(nextTheta)
        );

        glm::vec3 bottomLeft(
            radius * glm::cos(currentTheta),
            yBottom,
            radius * glm::sin(currentTheta)
        );

        glm::vec3 bottomRight(
            radius * glm::cos(nextTheta),
            yBottom,
            radius * glm::sin(nextTheta)
        );

        makeBodyTile(data, topLeft, topRight, bottomLeft, bottomRight);

    }
}


static void makeTopCapWedge(std::vector<float>& data,
                           int param1,
                           float currentTheta, float nextTheta) {

    const float radius = 0.5f;
    const float y = 0.5f;

    for (int i = 0; i < param1; i++) {

        float innerRadius = (float)i / param1 * radius;
        float outerRadius = (float)(i + 1) / param1 * radius;

        glm::vec3 innerStart(
            innerRadius * glm::cos(currentTheta),
            y,
            innerRadius * glm::sin(currentTheta)
        );

        glm::vec3 innerEnd(
            innerRadius * glm::cos(nextTheta),
            y,
            innerRadius * glm::sin(nextTheta)
        );

        glm::vec3 outerStart(
            outerRadius * glm::cos(currentTheta),
            y,
            outerRadius * glm::sin(currentTheta)
        );

        glm::vec3 outerEnd(
            outerRadius * glm::cos(nextTheta),
            y,
            outerRadius * glm::sin(nextTheta)
        );

        makeTopCapTile(data, innerStart, innerEnd, outerStart, outerEnd);

    }
}

static void makeBottomCapWedge(std::vector<float>& data,
                               int param1,
                               float currentTheta, float nextTheta) {

    const float radius = 0.5f;
    const float y = -0.5f;

    for (int i = 0; i < param1; i++) {

        float innerRadius = (float)i / param1 * radius;
        float outerRadius = (float)(i + 1) / param1 * radius;

        glm::vec3 innerStart(
            innerRadius * glm::cos(currentTheta),
            y,
            innerRadius * glm::sin(currentTheta)
            );

        glm::vec3 innerEnd(
            innerRadius * glm::cos(nextTheta),
            y,
            innerRadius * glm::sin(nextTheta)
            );

        glm::vec3 outerStart(
            outerRadius * glm::cos(currentTheta),
            y,
            outerRadius * glm::sin(currentTheta)
            );

        glm::vec3 outerEnd(
            outerRadius * glm::cos(nextTheta),
            y,
            outerRadius * glm::sin(nextTheta)
            );

        makeBottomCapTile(data, innerStart, innerEnd, outerStart, outerEnd);

    }
}

static void makeWedge(std::vector<float>& data,
                      int param1,
                      float currentTheta, float nextTheta) {

    makeBottomCapWedge(data, param1, currentTheta, nextTheta);
    makeBodyWedge(data, param1, currentTheta, nextTheta);
    makeTopCapWedge(data, param1, currentTheta, nextTheta);

}

static void makeCylinder(std::vector<float>& data,
                         int param1, int param2) {

    float thetaStep = glm::radians(360.0f / param2);

    for (int i = 0; i < param2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeWedge(data, param1, currentTheta, nextTheta);
    }

}

std::vector<float> Cylinder::generateCylinderData(int param1, int param2) {

    std::vector<float> data;
    param1 = glm::max(1, param1);
    param2 = glm::max(3, param2);

    data.reserve(3 * param1 * param2 * 2 * 3 * 6);
    makeCylinder(data, param1, param2);

    return data;
}

