#pragma once
#include <vector>
#include <GL/glew.h>

class Shape {

public:
    virtual ~Shape() = default;
    virtual std::vector<float> generateVertexData(int param1, int param2) = 0;

};
