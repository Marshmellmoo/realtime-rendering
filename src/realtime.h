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

    // =============================
    // OpenGL Resources
    // =============================
    
    GLuint m_defaultFBO;

    // === Shaders ===
    GLuint m_phong_shader;
    GLuint m_copy_shader;
    GLuint m_occlusion_shader;
    GLuint m_godrays_shader;
    GLuint m_fog_shader;
    GLuint m_grayscale_shader;
    GLuint m_blur_shader;
    GLuint m_vignette_shader;

    // === Framebuffers & Textures ===
    
    // Scene FBO
    GLuint m_scene_fbo;
    GLuint m_scene_color;
    GLuint m_scene_depth;

    // Occlusion FBO (for god rays)
    GLuint m_occlusion_fbo;
    GLuint m_occlusion_texture;
    GLuint m_occlusion_depth;

    // Fog FBO (reused for temp buffer)
    GLuint m_fog_fbo;
    GLuint m_fog_color;
    GLuint m_fog_depth;

    // Fullscreen Quad
    GLuint m_fullscreen_vao;
    GLuint m_fullscreen_vbo;

    // =============================
    // Initialization Functions
    // =============================
    
    void initializeFBO();
    void initializeOcclusionFBO();
    void initializeDepthFogFBO();
    void initializeFullscreenQuad();
    void initializeShapeGeometry();
    void initializeDepthBuffer();

    // =============================
    // Rendering Functions
    // =============================
    
    void render();
    void renderOcclusion();
    void renderShapesInstanced();
    void renderShapesNonInstanced();  // For performance comparison
    
    // Post-Processing
    void copy(GLuint texture);
    void blendDepthFog();
    void blendCrepuscular();
    void applyGrayscale(GLuint inputTexture);
    void applyBlur(GLuint inputTexture);
    void applyVignette(GLuint inputTexture);
    
    // Other
    void activateTextures(GLuint shader);
    void passLightsToShader(GLuint shader);
    void drawShape(const RenderShapeData& shape, GLuint shader);
    void passToDepthBuffer();
    void deleteFBOTextures();

    // =============================
    // Scene Data
    // =============================
    
    RenderData m_renderData;
    SceneGlobalData m_global;

    // === Camera ===
    Camera m_camera;
    glm::mat4 m_view;
    glm::mat4 m_projection;

    // === Shape Geometry ===
    bool geometryInit = false;
    int currParam1 = 1;
    bool currParam2 = 1;

    ShapeGeometry m_sphereGeometry;
    ShapeGeometry m_cubeGeometry;
    ShapeGeometry m_cylinderGeometry;
    ShapeGeometry m_coneGeometry;

    // =============================
    // Effect Parameters
    // =============================

    // God Rays
    bool m_enable_godrays;
    int m_godrays_samples;
    float m_godrays_density;
    float m_godrays_weight;
    float m_godrays_decay;
    float m_godrays_exposure;

    // Depth Fog
    bool m_enable_depth_fog;
    glm::vec3 m_fog_rgb;
    bool m_fog_relationship;        // True = Linear, False = Exponential
    float m_fog_maxdist;
    float m_fog_mindist;
    float m_fog_density;

    // Post-Process Effects
    bool m_grayscale_enabled;

    bool m_blur_enabled;
    float m_blur_radius;

    bool m_vignette_enabled;
    float m_vignette_strength;
    float m_vignette_extent;

    // Rendering Mode
    bool m_use_instanced_rendering;  // Toggle between instanced and non-instanced rendering

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
