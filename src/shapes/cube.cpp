#include "cube.h"
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

    glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(topLeft - bottomLeft), glm::vec3(topLeft - bottomRight)));

    insert(data, topLeft);
    insert(data, normal);
    insert(data, bottomLeft);
    insert(data, normal);
    insert(data, bottomRight);
    insert(data, normal);

    insert(data, topRight);
    insert(data, normal);
    insert(data, topLeft);
    insert(data, normal);
    insert(data, bottomRight);
    insert(data, normal);

}

static void makeFace(std::vector<float>& data,
              int param,
              glm::vec3 topLeft,
              glm::vec3 topRight,
              glm::vec3 bottomLeft,
              glm::vec3 bottomRight) {

    for (int row = 0; row < param; row++) {
        for (int col = 0; col < param; col++) {

            float a = (float)row / param;
            float b = (float)(row + 1)/ param;
            float c = (float)col / param;
            float d = (float)(col + 1) / param;

            glm::vec3 newTopLeft = topLeft + c * (topRight - topLeft) + a * (bottomLeft - topLeft);
            glm::vec3 newTopRight = topLeft + d * (topRight - topLeft) + a * (bottomLeft - topLeft);
            glm::vec3 newBottomLeft = topLeft + c * (topRight - topLeft) + b * (bottomLeft - topLeft);
            glm::vec3 newBottomRight = topLeft + d * (topRight - topLeft) + b * (bottomLeft - topLeft);

            makeTile(data, newTopLeft, newTopRight, newBottomLeft, newBottomRight);

        }
    }

}

static void makeCube(std::vector<float>& data, int param) {

    glm::vec3 corner[8] = {

    {-0.5f,  0.5f,  0.5f},  // 0. front-top-left !
        { 0.5f,  0.5f,  0.5f},  // 1. front-top-right !
        {-0.5f, -0.5f,  0.5f},  // 2. front-bottom-left !
        { 0.5f, -0.5f,  0.5f},  // 3. front-bottom-right !
        {-0.5f,  0.5f, -0.5f},  // 4. back-top-left !
        { 0.5f,  0.5f, -0.5f},  // 5. back-top-right !
        {-0.5f, -0.5f, -0.5f},  // 6. back-bottom-left !
        { 0.5f, -0.5f, -0.5f},  // 7. back-bottom-right !

    };

    makeFace(data, param, corner[0], corner[1], corner[2], corner[3]); // front-face
    makeFace(data, param, corner[6], corner[7], corner[4], corner[5]); // back-face

    makeFace(data, param, corner[4], corner[5], corner[0], corner[1]); // upward-face
    makeFace(data, param, corner[2], corner[3], corner[6], corner[7]); // downward-face

    makeFace(data, param, corner[4], corner[0], corner[6], corner[2]); // leftmost-face
    makeFace(data, param, corner[1], corner[5], corner[3], corner[7]); // rightmost-face

}


std::vector<float> Cube::generateCubeData(int param) {

    std::vector<float> data;
    param = glm::max(1, param);

    data.reserve(6 * param * param * 2 * 6 * 6);
    makeCube(data, param);

    return data;
}
