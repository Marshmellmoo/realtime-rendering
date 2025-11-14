#include "sphere.h"
#include <glm/glm.hpp>

static void insert(std::vector<float>& data, glm::vec3 v) {

    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);

}

static void makeTile(std::vector<float>& data,
              glm::vec3 topLeft,
              glm::vec3 topRight,
              glm::vec3 bottomLeft,
              glm::vec3 bottomRight) {

    glm::vec3 normalTL = glm::normalize(topLeft);
    glm::vec3 normalTR = glm::normalize(topRight);
    glm::vec3 normalBL = glm::normalize(bottomLeft);
    glm::vec3 normalBR = glm::normalize(bottomRight);

    insert(data, topLeft); insert(data, normalTL);
    insert(data, bottomLeft); insert(data, normalBL);
    insert(data, bottomRight); insert(data, normalBR);

    insert(data, bottomRight); insert(data, normalBR);
    insert(data, topRight); insert(data, normalTR);
    insert(data, topLeft); insert(data, normalTL);

}

static void makeWedge(std::vector<float>& data,
               int param1,
               float currentTheta,
               float nextTheta) {

    float r = 0.5f;
    for (int i = 0; i < param1; i++) {

        float phiTop = i * glm::radians(180.0f / param1);
        float phiBottom = (i + 1) * glm::radians(180.0f / param1);

        glm::vec3 topLeft = glm::vec3(r * glm::sin(phiTop) * glm::cos(currentTheta),
                                      r * glm::cos(phiTop),
                                      -r * glm::sin(phiTop) * glm::sin(currentTheta));

        glm::vec3 topRight = glm::vec3(r * glm::sin(phiTop) * glm::cos(nextTheta),
                                       r * glm::cos(phiTop),
                                       -r * glm::sin(phiTop) * glm::sin(nextTheta));

        glm::vec3 bottomLeft = glm::vec3(r * glm::sin(phiBottom) * glm::cos(currentTheta),
                                         r * glm::cos(phiBottom),
                                         -r * glm::sin(phiBottom) * glm::sin(currentTheta));

        glm::vec3 bottomRight = glm::vec3(r * glm::sin(phiBottom) * glm::cos(nextTheta),
                                          r * glm::cos(phiBottom),
                                          -r * glm::sin(phiBottom) * glm::sin(nextTheta));

        makeTile(data, topLeft, topRight, bottomLeft, bottomRight);

    }

}

static void makeSphere(std::vector<float>& data, int param1, int param2) {

    for (int i = 0; i < param2; i++) {

        float thetaLeft = i * glm::radians(360.0f) / param2;
        float thetaRight = (i + 1) * glm::radians(360.0f) / param2;
        makeWedge(data, param1, thetaLeft, thetaRight);

    }

}

std::vector<float> Sphere::generateSphereData(int param1, int param2) {

    std::vector<float> data;
    param1 = glm::max(1, param1);
    param2 = glm::max(1, param2);

    data.reserve(6 * param1 * param2 * 2 * 6 * 6);
    makeSphere(data, param1, param2);

    return data;

}
