#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "camera/camera.h"
#include "utils/sceneparser.h"
#include "utils/shaderloader.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "shaders/fbo.h"//;

struct ShapeGeometry {

    GLuint vbo;
    GLuint vao;
    int verticies;

    // Instancing
    GLuint instanceVBO;
    GLuint materialVBO;

};


class Realtime : public QOpenGLWidget {

public:  
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots: 
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:  
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private: 
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    GLuint m_defaultFBO;

    // === Scene Data ===
    RenderData m_renderData;
    SceneGlobalData m_global;

    void render();
    void copy(GLuint text);

    void renderOcclusion();
    void blendCrepuscular();
    void blendDepthFog();

    void activateTextures(GLuint shader);

    // === Camera ===
    Camera m_camera;
    glm::mat4 m_view;
    glm::mat4 m_projection;


    // === Shaders ===
    GLuint m_phong_shader;
    GLuint m_copy_shader;
    GLuint m_occlusion_shader;
    GLuint m_godrays_shader;
    GLuint m_fog_shader;

    // Scene FBO
    GLuint m_scene_color;
    GLuint m_scene_depth;
    GLuint m_scene_fbo;

    // Fog FBO
    GLuint m_fog_fbo;
    GLuint m_fog_color;
    GLuint m_fog_depth;

    // Occlusion FBO
    GLuint m_occlusion_fbo;
    GLuint m_occlusion_texture;
    GLuint m_occlusion_depth;

    // Fullscreen Quad
    GLuint m_fullscreen_vao;
    GLuint m_fullscreen_vbo;

    void initializeFBO();
    void initializeOcclusionFBO();
    void initializeDepthFogFBO();
    void initializeFullscreenQuad();

    // ==========
    // Parameters
    // ==========

    // Godrays
    bool m_enable_godrays;
    int m_godrays_samples;

    float m_godrays_density;
    float m_godrays_weight;
    float m_godrays_decay;
    float m_godrays_exposure;

    // Depth Fog
    bool m_enable_depth_fog;

    glm::vec3 m_fog_rgb;
    bool m_fog_relationship; // True = Linear, False = Exponential
    float m_fog_maxdist;
    float m_fog_mindist;
    float m_fog_density;  // For exponential fog


    void initializeDepthBuffer();
    void passToDepthBuffer();
    void deleteFBOTextures();

    // === Shape Information ===
    bool geometryInit = false;
    int currParam1 = 1;
    bool currParam2 = 1;

    ShapeGeometry m_sphereGeometry;
    ShapeGeometry m_cubeGeometry;
    ShapeGeometry m_cylinderGeometry;
    ShapeGeometry m_coneGeometry;

    void initializeShapeGeometry();
    void passLightsToShader(GLuint shader);
    void drawShape(const RenderShapeData& shape, GLuint shader);
    void renderShapesInstanced();

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    double m_devicePixelRatio;
};
