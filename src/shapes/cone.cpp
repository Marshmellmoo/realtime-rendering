#pragma once
#include "cone.h"
#include <glm/glm.hpp>

static void insert(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

static glm::vec3 calculateNormal(glm::vec3 &pt) {
    float x = 2.f * pt.x;
    float y = -(0.5f) * (2.f * pt.y - 1.f);
    float z = 2.f * pt.z;

    glm::vec3 normal(x, y, z);
    return glm::normalize(normal);
}

static glm::vec3 computeSlopeNormal(glm::vec3 &pt, glm::vec3 &edgeA, glm::vec3 &edgeB) {
    float radius = glm::length(glm::vec2(pt.x, pt.z));

    if (radius < 1e-5f) {
        glm::vec3 n1 = calculateNormal(edgeA);
        glm::vec3 n2 = calculateNormal(edgeB);
        return glm::normalize(n1 + n2);
    }

    return calculateNormal(pt);
}

static void makeSideTile(std::vector<float>& data,
                  glm::vec3 topStart, glm::vec3 topEnd,
                  glm::vec3 bottomStart, glm::vec3 bottomEnd) {

    glm::vec3 normalTL = computeSlopeNormal(topStart, bottomStart, bottomEnd);
    glm::vec3 normalTR = computeSlopeNormal(topEnd, bottomStart, bottomEnd);
    glm::vec3 normalBL = calculateNormal(bottomStart);
    glm::vec3 normalBR = calculateNormal(bottomEnd);

    // First triangle
    insert(data, bottomStart);
    insert(data, normalBL);

    insert(data, topStart);
    insert(data, normalTL);

    insert(data, topEnd);
    insert(data, normalTR);

    // Second triangle
    insert(data, bottomStart);
    insert(data, normalBL);

    insert(data, topEnd);
    insert(data, normalTR);

    insert(data, bottomEnd);
    insert(data, normalBR);

}

static void makeBaseTile(std::vector<float> data,
                  glm::vec3 innerStart, glm::vec3 innerEnd,
                  glm::vec3 outerStart, glm::vec3 outerEnd) {

    glm::vec3 normal(0.f, -1.f, 0.f);

    // Triangle 1
    insert(data, outerStart);
    insert(data, normal);

    insert(data, innerEnd);
    insert(data, normal);

    insert(data, innerStart);
    insert(data, normal);

    // Triangle 2
    insert(data, outerStart);
    insert(data, normal);

    insert(data, outerEnd);
    insert(data, normal);

    insert(data, innerEnd);
    insert(data, normal);
}

static void buildBaseSegment(std::vector<float>& data,
                      int param1,
                      float startTheta, float endTheta) {

    const float radius = 0.5f;
    const float y = -0.5f;

    for (int i = 0; i < param1; i++) {

        float innerRadius = (float)i / param1 * radius;
        float outerRadius = (float)(i + 1) / param1 * radius;

        glm::vec3 innerStart(innerRadius * glm::cos(startTheta), y, innerRadius * glm::sin(startTheta));
        glm::vec3 innerEnd(innerRadius * glm::cos(endTheta),     y, innerRadius * glm::sin(endTheta));
        glm::vec3 outerStart(outerRadius * glm::cos(startTheta), y, outerRadius * glm::sin(startTheta));
        glm::vec3 outerEnd(outerRadius * glm::cos(endTheta),     y, outerRadius * glm::sin(endTheta));

        makeBaseTile(data, innerStart, innerEnd, outerStart, outerEnd);
    }
}

static void buildSideSegment(std::vector<float>& data,
                      int param1,
                      float startTheta, float endTheta) {
    const float totalHeight = 1.f;
    const float halfHeight = totalHeight / 2.f;
    const float baseRadius = 0.5f;

    for (int row = 0; row < param1; row++) {
        float yTop = -halfHeight + (row + 1) * (totalHeight / param1);
        float yBottom = -halfHeight + row * (totalHeight / param1);

        float topRadius = baseRadius * (1.f - (float)(row + 1) / param1);
        float bottomRadius = baseRadius * (1.f - (float)row / param1);

        glm::vec3 topStart(topRadius * glm::cos(startTheta), yTop, topRadius * glm::sin(startTheta));
        glm::vec3 topEnd(topRadius * glm::cos(endTheta),     yTop, topRadius * glm::sin(endTheta));
        glm::vec3 bottomStart(bottomRadius * glm::cos(startTheta), yBottom, bottomRadius * glm::sin(startTheta));
        glm::vec3 bottomEnd(bottomRadius * glm::cos(endTheta),     yBottom, bottomRadius * glm::sin(endTheta));

        makeSideTile(data, topStart, topEnd, bottomStart, bottomEnd);
    }
}

static void makeWedge(std::vector<float>& data,
               int param1,
               float startTheta, float endTheta) {

    buildBaseSegment(data, param1, startTheta, endTheta);
    buildSideSegment(data, param1, startTheta, endTheta);

}

static void makeCone(std::vector<float>& data, int param1, int param2) {

    data.clear();
    float thetaStep = glm::radians(360.f / static_cast<float>(param2));

    for (int i = 0; i < param2; i++) {
        float startTheta = i * thetaStep;
        float endTheta = (i + 1) * thetaStep;
        makeWedge(data, param1, startTheta, endTheta);
    }

}

std::vector<float> Cone::generateConeData(int param1, int param2) {

    std::vector<float> data;
    param1 = glm::max(1, param1);
    param2 = glm::max(1, param2);

    data.reserve(6 * param1 * param2 * 2 * 6 * 6);
    makeCone(data, param1, param2);

    return data;

}
