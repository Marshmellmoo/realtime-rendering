#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

void nodeTraversal(SceneNode* node, RenderData &renderData, glm::mat4 ctm) {

    if (node == NULL) {
        return;
    }

    glm::mat4 culCTM = glm::mat4(1.0f);

    for (SceneTransformation* transform : node->transformations) {
        switch (transform->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            culCTM *= glm::translate(glm::mat4(1.0f), transform->translate);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            culCTM *= glm::rotate(glm::mat4(1.0f), transform->angle, transform->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            culCTM *= glm::scale(glm::mat4(1.0f), transform->scale);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            culCTM *= transform->matrix;
            break;
        }
    }

    ctm *= culCTM;

    for (ScenePrimitive* prim : node->primitives) {
        RenderShapeData primitive = {*prim, ctm};
        renderData.shapes.push_back(primitive);
    }

    for (SceneLight* light : node->lights) {
        glm::vec4 lightPos = {0, 0, 0, 1};
        SceneLightData lighting = {light->id, light->type, light->color, light->function, ctm * lightPos, ctm * light->dir, light->penumbra, light->angle, light->width, light->height};
        renderData.lights.push_back(lighting);
    }

    for (SceneNode* child : node->children) {
        nodeTraversal(child, renderData, ctm);
    }

}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);

    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // Task 5: populate renderData with global data, and camera data;
    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    // Task 6: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!

    auto rootNode = fileReader.getRootNode();
    renderData.shapes.clear();
    renderData.lights.clear();

    nodeTraversal(rootNode, renderData, glm::mat4(1.0f));

    return true;

}

