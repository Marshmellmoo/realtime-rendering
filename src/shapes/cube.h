#include <glm/glm.hpp>
#include "shapes/shape.h"

class Cube : public Shape {

public:
    static std::vector<float> generateCubeData(int param);

};
