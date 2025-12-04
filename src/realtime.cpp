#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "settings.h"

#include "utils/shaderloader.h"

#include "shapes/cube.h"
#include "shapes/sphere.h"
#include "shapes/cone.h"
#include "shapes/cylinder.h"

// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
    m_view = glm::mat4(1.0f);
    m_projection = glm::mat4(1.0f);

    // Initialize global lighting with default values
    m_global.ka = 0.5f;
    m_global.kd = 0.5f;
    m_global.ks = 0.5f;



}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_defaultFBO = 4;

    m_phong_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/default.vert",
        ":/resources/shaders/default.frag"
        );

    m_occlusion_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/occlusion.vert",
        ":/resources/shaders/occlusion.frag"
        );

    // Validate occlusion shader
    if (m_occlusion_shader == 0) {
        std::cerr << "[ERROR] Failed to create occlusion shader program (returned 0)" << std::endl;
    } else {
        GLint linkStatus = 0;
        glGetProgramiv(m_occlusion_shader, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            std::cerr << "[ERROR] Occlusion shader program link failed" << std::endl;
            GLint logLen = 0;
            glGetProgramiv(m_occlusion_shader, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 0) {
                std::vector<GLchar> logText(logLen);
                glGetProgramInfoLog(m_occlusion_shader, logLen, nullptr, logText.data());
                std::cerr << "Link log: " << logText.data() << std::endl;
            }
        }
    }

    m_copy_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/copy.vert",
        ":/resources/shaders/copy.frag");

    m_godrays_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/godrays.vert",
        ":/resources/shaders/godrays.frag");

    // m_fog_shader = ShaderLoader::createShaderProgram(
    //     ":resources/shaders/depth_fog.vert",
    //     ":resources/shaders/depth_fog.frag");

    initializeFBO();
    initializeOcclusionFBO();
    initializeDepthFogFBO();

    initializeFullscreenQuad();
    initializeShapeGeometry();

    geometryInit = true;

    m_enable_godrays = true;
    m_godrays_samples = 100;    // Number of samples for radial blur
    m_godrays_density = 1.0f;   // Ray spread density
    m_godrays_weight = 0.01f;   // Weight per sample (matches reference)
    m_godrays_decay = 1.0f;     // Decay factor between samples (matches reference)
    m_godrays_exposure = 1.0f;  // Exposure for visibility (matches reference)

    m_fog_maxdist = 1.0;
    m_fog_mindist = 0.5;
    m_fog_rgb = glm::vec3(0.5, 0.5, 0.5);
    m_enable_depth_fog = true;

}

void Realtime::initializeFBO() {

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    glGenFramebuffers(1, &m_scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_scene_fbo);

    glGenTextures(1, &m_scene_color);
    glBindTexture(GL_TEXTURE_2D, m_scene_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_scene_depth);
    glBindTexture(GL_TEXTURE_2D, m_scene_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_scene_color, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_scene_depth, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

    // bind to related fbo, pass color tture val to shader, unbind fbo

}

void Realtime::initializeDepthFogFBO() {

    int width = (size().width() * m_devicePixelRatio);
    int height = (size().height() * m_devicePixelRatio);

    glGenFramebuffers(1, &m_fog_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fog_fbo);

    // Create color texture FIRST
    glGenTextures(1, &m_fog_color);
    glBindTexture(GL_TEXTURE_2D, m_fog_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create depth texture SECOND
    glGenTextures(1, &m_fog_depth);
    glBindTexture(GL_TEXTURE_2D, m_fog_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // NOW attach them
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_fog_color, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_fog_depth, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Realtime::initializeOcclusionFBO() {
    // Use a lower-resolution occlusion map (half resolution) for better performance
    int width = (size().width() * m_devicePixelRatio) / 2;
    int height = (size().height() * m_devicePixelRatio) / 2;

    if (width < 1) width = 1;
    if (height < 1) height = 1;

    glGenTextures(1, &m_occlusion_texture);
    glBindTexture(GL_TEXTURE_2D, m_occlusion_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenRenderbuffers(1, &m_occlusion_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_occlusion_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glGenFramebuffers(1, &m_occlusion_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_occlusion_texture, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_occlusion_depth);

    // Ensure framebuffer is complete in debug builds (optional)
#ifndef NDEBUG
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Occlusion FBO incomplete: " << status << std::endl;
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Realtime::initializeFullscreenQuad() {

    std::vector<GLfloat> quadData = {

    -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f

};

glGenVertexArrays(1, &m_fullscreen_vao);
glGenBuffers(1, &m_fullscreen_vbo);

glBindVertexArray(m_fullscreen_vao);
glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);

glBufferData(GL_ARRAY_BUFFER,
             quadData.size() * sizeof(GLfloat),
             quadData.data(),
             GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                      4 * sizeof(GLfloat),
                      (void *)0);

glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                      4 * sizeof(GLfloat),
                      (void *)(2 * sizeof(GLfloat)));

glBindVertexArray(0);
bool debugShowOcclusion = false; // disabled by default to allow normal compositing

}

void Realtime::initializeShapeGeometry() {

    int param1 = settings.shapeParameter1;
    int param2 = settings.shapeParameter2;

    // Initializing Sphere VBOs & VAOs --
    std::vector<float> sphere = Sphere::generateSphereData(param1, param2);

    glGenBuffers(1, &m_sphereGeometry.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereGeometry.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sphere.size() * sizeof(GLfloat),
                 sphere.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_sphereGeometry.vao);
    glBindVertexArray(m_sphereGeometry.vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)(3 * sizeof(GLfloat)));

    m_sphereGeometry.verticies = sphere.size() / 6;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Initializing Cube VBOs & VAOs  --
    std::vector<float> cube = Cube::generateCubeData(param1);

    glGenBuffers(1, &m_cubeGeometry.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeGeometry.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 cube.size() * sizeof(GLfloat),
                 cube.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_cubeGeometry.vao);
    glBindVertexArray(m_cubeGeometry.vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)(3 * sizeof(GLfloat)));

    // Vertex Count
    m_cubeGeometry.verticies = cube.size() / 6;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Initializing Cylinder VBOs & VAOs --
    std::vector<float> cylinder = Cylinder::generateCylinderData(param1, param2);

    glGenBuffers(1, &m_cylinderGeometry.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_cylinderGeometry.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 cylinder.size() * sizeof(GLfloat),
                 cylinder.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_cylinderGeometry.vao);
    glBindVertexArray(m_cylinderGeometry.vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat),
                          (void*)(3 * sizeof(GLfloat)));

    // Vertex Count
    m_cylinderGeometry.verticies = cylinder.size() / 6;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    // Initializing Cone VBOs & VAOs --
    std::vector<float> cone = Cone::generateConeData(param1, param2);

    glGenBuffers(1, &m_coneGeometry.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_coneGeometry.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 cone.size() * sizeof(GLfloat),
                 cone.data(),
                 GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_coneGeometry.vao);
    glBindVertexArray(m_coneGeometry.vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    m_coneGeometry.verticies = cone.size() / 6;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Realtime::paintGL() {

    int width  = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    // =============================================
    // PASS 1: Occlusion Pre-Pass
    // =============================================
    // Render geometry black, lights white to half-res occlusion FBO
    if (m_enable_godrays) {
        renderOcclusion();
    }

    // =============================================
    // PASS 2: Main Scene Render
    // =============================================
    // Render normally to default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, width, height);
    glClearColor(0.05f, 0.0f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ensure blending is disabled for opaque geometry rendering
    glDisable(GL_BLEND);
    glUseProgram(m_phong_shader);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "view"),
                       1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "proj"),
                       1, GL_FALSE, &m_projection[0][0]);

    glm::vec3 camPosition = m_camera.getInverseViewMatrix()[3];
    glUniform3fv(glGetUniformLocation(m_phong_shader, "cameraPosition"),
                 1, &camPosition[0]);

    glUniform1f(glGetUniformLocation(m_phong_shader, "ka"), m_global.ka);
    glUniform1f(glGetUniformLocation(m_phong_shader, "kd"), m_global.kd);
    glUniform1f(glGetUniformLocation(m_phong_shader, "ks"), m_global.ks);

    passLightsToShader(m_phong_shader);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    for (const auto& shape : m_renderData.shapes) {
        drawShape(shape, m_phong_shader);
    }

    // =============================================
    // PASS 3: Post-Processing God Rays
    // =============================================
    // Apply godrays WITHOUT clearing framebuffer (blend onto rendered scene)
    if (m_enable_godrays) {
        glDisable(GL_DEPTH_TEST);  // Disable depth test for post-process
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);
        blendCrepuscular();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);  // Re-enable for consistency
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

}

void Realtime::render() {

    glBindFramebuffer(GL_FRAMEBUFFER, m_scene_fbo);

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_phong_shader);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "view"),
                       1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong_shader, "proj"),
                       1, GL_FALSE, &m_projection[0][0]);

    glm::vec3 camPosition = m_camera.getInverseViewMatrix()[3];
    glUniform3fv(glGetUniformLocation(m_phong_shader, "cameraPosition"),
                 1, &camPosition[0]);

    glUniform1f(glGetUniformLocation(m_phong_shader, "ka"),
                m_global.ka);
    glUniform1f(glGetUniformLocation(m_phong_shader, "kd"),
                m_global.kd);
    glUniform1f(glGetUniformLocation(m_phong_shader, "ks"),
                m_global.ks);

    passLightsToShader(m_phong_shader);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    for (const auto& shape : m_renderData.shapes) {
        drawShape(shape, m_phong_shader);
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Realtime::copy() {

    int width  = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_copy_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_scene_color);
    glUniform1i(glGetUniformLocation(m_copy_shader, "sceneTexture"), 0);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);

}

void Realtime::renderOcclusion() {

    glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_fbo);

    int width = (size().width() * m_devicePixelRatio) / 2;
    int height = (size().height() * m_devicePixelRatio) / 2;
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_occlusion_shader);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_occlusion_shader, "view"),
                       1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_occlusion_shader, "proj"),
                       1, GL_FALSE, &m_projection[0][0]);

    // Render all geometry as black
    GLint colorLoc = glGetUniformLocation(m_occlusion_shader, "occlusionColor");
    glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);

    int shapeCount = 0;
    for (const auto& shape : m_renderData.shapes) {
        shapeCount++;
        glUniformMatrix4fv(glGetUniformLocation(m_occlusion_shader, "model"),
                           1, GL_FALSE, &shape.ctm[0][0]);
        
        GLuint vao;
        int verticies;

        switch(shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_CUBE:
            vao = m_cubeGeometry.vao;
            verticies = m_cubeGeometry.verticies;
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            vao = m_sphereGeometry.vao;
            verticies = m_sphereGeometry.verticies;
            break;
        case PrimitiveType::PRIMITIVE_CONE:
            vao = m_coneGeometry.vao;
            verticies = m_coneGeometry.verticies;
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            vao = m_cylinderGeometry.vao;
            verticies = m_cylinderGeometry.verticies;
            break;
        case PrimitiveType::PRIMITIVE_MESH:
            continue;
        default:
            continue;
        }
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, verticies);
        glBindVertexArray(0);
    }
    
    std::cout << "[OCCLUSION] Drew " << shapeCount << " shapes" << std::endl;

    // Render light sources as white
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    int lightCount = 0;

    // Render light geometry into the occlusion map **without depth testing** so
    // the occlusion texture contains a white marker at the light's screen
    // position even if the light model is behind other geometry. This makes
    // the radial blur originate from the correct screen location and allows
    // rays to "peak through" occluders.
    glDisable(GL_DEPTH_TEST);
    for (const auto& light : m_renderData.lights) {
        lightCount++;
        glm::mat4 lightModel = glm::mat4(1.0f);

        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            glm::vec3 lightDir = glm::normalize(glm::vec3(light.dir));
            // Place the light sphere 100 units along the direction vector
            glm::vec3 lightWorldPos = lightDir * 100.0f;
            lightModel = glm::translate(lightModel, lightWorldPos);
            lightModel = glm::scale(lightModel, glm::vec3(15.0f));
        } else if (light.type == LightType::LIGHT_POINT) {
            // Point lights: render at actual position with smaller radius
            lightModel = glm::translate(lightModel, glm::vec3(light.pos));
            lightModel = glm::scale(lightModel, glm::vec3(0.5f));
        } else {
            // Spot lights: render at actual position
            lightModel = glm::translate(lightModel, glm::vec3(light.pos));
            lightModel = glm::scale(lightModel, glm::vec3(0.3f));
        }

        glUniformMatrix4fv(glGetUniformLocation(m_occlusion_shader, "model"),
                           1, GL_FALSE, &lightModel[0][0]);

        // Render light as a sphere (no depth test)
        glBindVertexArray(m_sphereGeometry.vao);
        glDrawArrays(GL_TRIANGLES, 0, m_sphereGeometry.verticies);
        glBindVertexArray(0);
    }
    // restore depth testing for subsequent passes
    glEnable(GL_DEPTH_TEST);
    std::cout << "[OCCLUSION] Drew " << lightCount << " lights" << std::endl;

    glUseProgram(0);
    
    // DEBUG: Read a pixel to verify occlusion FBO has content
    // Note: viewport is half-res, so read from center of that half-res texture
    {
        int occlusionWidth = width;  // already set to size().width() / 2
        int occlusionHeight = height; // already set to size().height() / 2
        unsigned char pixel[3] = {0, 0, 0};
        glReadPixels(occlusionWidth/2, occlusionHeight/2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
        std::cout << "[OCCLUSION] Center pixel: R=" << (int)pixel[0] << " G=" << (int)pixel[1] << " B=" << (int)pixel[2] << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    
    // Reset viewport to main window
    int mainWidth = size().width() * m_devicePixelRatio;
    int mainHeight = size().height() * m_devicePixelRatio;
    glViewport(0, 0, mainWidth, mainHeight);

}

void Realtime::blendDepthFog() {

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glUseProgram(m_fog_shader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_scene_color);
    glUniform1i(glGetUniformLocation(m_fog_shader, "sceneTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_scene_depth);
    glUniform1i(glGetUniformLocation(m_fog_shader, "depthTexture"), 1);

    glUniform1f(glGetUniformLocation(m_fog_shader, "minDist"), m_fog_mindist);
    glUniform1f(glGetUniformLocation(m_fog_shader, "maxDist"), m_fog_maxdist);
    glUniform3fv(glGetUniformLocation(m_fog_shader, "fogColour"), 1, &m_fog_rgb[0]);

    glBindVertexArray(m_fullscreen_vao);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

}

void Realtime::blendCrepuscular() {

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    glViewport(0, 0, width, height);

    // DEBUG: Display occlusion texture to verify it has content
    bool debugOcclusion = false;  // Set to true to visualize occlusion map
    if (debugOcclusion) {
        glDisable(GL_BLEND);
        glUseProgram(m_copy_shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_occlusion_texture);
        glUniform1i(glGetUniformLocation(m_copy_shader, "sceneTexture"), 0);
        glBindVertexArray(m_fullscreen_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);
        glEnable(GL_BLEND);
        return;
    }

    glUseProgram(m_godrays_shader);

    // Bind occlusion texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_occlusion_texture);
    glUniform1i(glGetUniformLocation(m_godrays_shader, "occlusionTexture"), 0);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    // Calculate screen-space light positions from view-projection matrix
    std::vector<glm::vec4> light_positions;
    for (const auto& light : m_renderData.lights) {

        glm::vec3 lightWorldPosition;

        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            // Use fixed world position for directional light (same as in renderOcclusion)
            glm::vec3 lightDir = glm::normalize(glm::vec3(light.dir));
            lightWorldPosition = lightDir * 100.0f;
        } else if (light.type == LightType::LIGHT_POINT) {
            lightWorldPosition = glm::vec3(light.pos);
        } else {
            lightWorldPosition = glm::vec3(light.pos);
        }

        // Transform exactly like reference: view_projection * light_position
        const glm::vec4 clip_light_position = m_projection * m_view * glm::vec4(lightWorldPosition, 1.0f);
        const glm::vec4 ndc_light_position = clip_light_position / clip_light_position.w;
        const glm::vec4 screen_space_light_position = (ndc_light_position + 1.0f) * 0.5f;
        light_positions.emplace_back(screen_space_light_position);
    }

    // Set blur parameters
    glUniform1i(glGetUniformLocation(m_godrays_shader, "blurParams.sampleCount"),
                m_godrays_samples);
    glUniform1f(glGetUniformLocation(m_godrays_shader, "blurParams.blurDensity"),
                m_godrays_density);
    glUniform1f(glGetUniformLocation(m_godrays_shader, "blurParams.sampleWeight"),
                m_godrays_weight);
    glUniform1f(glGetUniformLocation(m_godrays_shader, "blurParams.decayFactor"),
                m_godrays_decay);
    glUniform1f(glGetUniformLocation(m_godrays_shader, "blurParams.blurExposure"),
                m_godrays_exposure);

    // Set light positions array
    if (!light_positions.empty()) {
        glUniform4fv(glGetUniformLocation(m_godrays_shader, "lightPositionsScreen"),
                     light_positions.size(), glm::value_ptr(light_positions[0]));
    }
    glUniform1i(glGetUniformLocation(m_godrays_shader, "lightCount"),
                (int)light_positions.size());

    // Draw fullscreen quad
    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glUseProgram(0);

}

void Realtime::passLightsToShader(GLuint shader) {

    auto info = m_renderData.lights;

    glUniform1i(glGetUniformLocation(shader, "lightCount"),
                info.size());

    for (size_t i = 0; i < info.size(); i++) {

        int type;
        const SceneLightData& light = info[i];
        std::string base = "lights[" + std::to_string(i) + "]";

        switch (light.type) {

        case LightType::LIGHT_DIRECTIONAL:
            type = 0;
            break;

        case LightType::LIGHT_POINT:
            type = 1;
            break;

        case LightType::LIGHT_SPOT:
            type = 2;
            break;

        }

        // Initializing Light Type =
        glUniform1i(glGetUniformLocation(shader, (base + ".type").c_str()),
                    type);

        // Initializing Color =
        glm::vec4 color = light.color;
        glUniform4fv(glGetUniformLocation(shader, (base + ".color").c_str()),
                     1, &color[0]);

        // Initializing Function =
        glm::vec3 function = light.function;
        glUniform3fv(glGetUniformLocation(shader, (base + ".function").c_str()),
                     1, &function[0]);

        // Initializing Position =
        if (type == 1 || type == 2) {

            glm::vec4 position = light.pos;
            glUniform4fv(glGetUniformLocation(shader, (base + ".position").c_str()),
                         1, &position[0]);

        }


        // Initializing Direction =
        if (type == 0 || type == 2) {

            glm::vec4 direction = light.dir;
            glUniform4fv(glGetUniformLocation(shader, (base + ".direction").c_str()),
                         1, &direction[0]);

        }

        // Initializing Penumbra & Angle =
        if (type == 2) {

            float penumbra = light.penumbra;
            float angle = light.angle;

            glUniform1f(glGetUniformLocation(shader, (base + ".penumbra").c_str()), penumbra);
            glUniform1f(glGetUniformLocation(shader, (base + ".angle").c_str()), angle);

        }

    }

}

void Realtime::drawShape(const RenderShapeData& shape, GLuint shader) {

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"),
                       1, GL_FALSE, &shape.ctm[0][0]);

    GLint invModelLoc = glGetUniformLocation(shader, "invModel");
    if (invModelLoc != -1) {

        glm::mat4 invModel = glm::inverse(shape.ctm);
        glUniformMatrix4fv(invModelLoc, 1, GL_FALSE, &invModel[0][0]);

    }

    GLint ambientLoc = glGetUniformLocation(shader, "c_ambient");
    GLint diffuseLoc = glGetUniformLocation(shader, "c_diffuse");
    GLint specularLoc = glGetUniformLocation(shader, "c_specular");
    GLint shininessLoc = glGetUniformLocation(shader, "shininess");

    if (ambientLoc != -1 || diffuseLoc != -1 || specularLoc != -1 || shininessLoc != -1) {

        auto info = shape.primitive.material;

        glm::vec4 ambient = info.cAmbient;
        glm::vec4 diffuse = info.cDiffuse;
        glm::vec4 specular = info.cSpecular;

        if (ambientLoc != -1) glUniform4fv(ambientLoc, 1, &ambient[0]);
        if (diffuseLoc != -1) glUniform4fv(diffuseLoc, 1, &diffuse[0]);
        if (specularLoc != -1) glUniform4fv(specularLoc, 1, &specular[0]);
        if (shininessLoc != -1) glUniform1f(shininessLoc, info.shininess);

    }

    GLuint vao;
    int verticies;

    switch(shape.primitive.type) {

    case PrimitiveType::PRIMITIVE_CUBE:
        vao = m_cubeGeometry.vao;
        verticies = m_cubeGeometry.verticies;
        break;

    case PrimitiveType::PRIMITIVE_SPHERE:
        vao = m_sphereGeometry.vao;
        verticies = m_sphereGeometry.verticies;
        break;

    case PrimitiveType::PRIMITIVE_CONE:
        vao = m_coneGeometry.vao;
        verticies = m_coneGeometry.verticies;
        break;

    case PrimitiveType::PRIMITIVE_CYLINDER:
        vao = m_cylinderGeometry.vao;
        verticies = m_cylinderGeometry.verticies;
        break;

    case PrimitiveType::PRIMITIVE_MESH:
        return;
        break;

    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, verticies);
    glBindVertexArray(0);

    // call bindframebuffer(gl_framebuffer, 0) before final pass / blend

}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    float aspectRatio = (float)w / (float)h;
    m_camera.setAspectRatio(aspectRatio);

    // initializeDepthBuffer();

}

void Realtime::deleteFBOTextures() {

    // int width = size().width() * m_devicePixelRatio;
    // int height = size().height() * m_devicePixelRatio;

    // glDeleteTextures(1, &m_color);
    // // glDeleteTextures(1, &m_depth);

    // glGenTextures(1, &m_color);
    // glBindTexture(GL_TEXTURE_2D, m_color);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
    //              GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // // glGenTextures(1, &m_depth);
    // // glBindTexture(GL_TEXTURE_2D, m_depth);
    // // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
    // //              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    //                        GL_TEXTURE_2D, m_color, 0);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
    //                        GL_TEXTURE_2D, m_depth, 0);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Realtime::sceneChanged() {

    // Get the scene file path
    std::string sceneFilepath = settings.sceneFilePath;

    if (sceneFilepath.empty()) {
        std::cout << "No scene file loaded" << std::endl;
        return;
    }

    std::cout << "Loading scene: " << sceneFilepath << std::endl;

    // Parse the scene
    bool success = SceneParser::parse(sceneFilepath, m_renderData);

    if (!success) {
        std::cerr << "Failed to parse scene file." << std::endl;
        return;
    }

    m_global = m_renderData.globalData;

    auto cam = m_renderData.cameraData;

    m_camera.setPosition(cam.pos);
    m_camera.setLook(cam.look);
    m_camera.setUp(cam.up);

    m_camera.setAspectRatio((float)(size().width()) / (float)(size().height()));
    m_camera.setAngles(cam.heightAngle);

    m_camera.setNearPlane(settings.nearPlane);
    m_camera.setFarPlane(settings.farPlane);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    currParam1 = settings.shapeParameter1;
    currParam2 = settings.shapeParameter2;

    initializeFBO();
    initializeOcclusionFBO();

    initializeShapeGeometry();
    initializeFullscreenQuad();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {

    m_camera.setNearPlane(settings.nearPlane);
    m_camera.setFarPlane(settings.farPlane);

    // Checks if the camera has moved within the camera object .
    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    if (geometryInit && (currParam1 != settings.shapeParameter1 || currParam2 != settings.shapeParameter1)) {
        initializeShapeGeometry();
    }

    update(); // asks for a PaintGL() call to occur

}


// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {

    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        float sensitivity = 0.002f;
        m_camera.rotate(deltaX * sensitivity, deltaY * sensitivity);
        m_view = m_camera.getViewMatrix();

        update();
    }

}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    float speed;
    glm::vec3 movement(0.0f);

    speed = 5.0f * deltaTime;

    glm::vec3 look = glm::normalize(glm::vec3(m_camera.getInverseViewMatrix()[2]));
    glm::vec3 right = glm::normalize(glm::vec3(m_camera.getInverseViewMatrix()[0]));
    glm::vec3 up = glm::vec3(0, 1, 0);

    if (m_keyMap[Qt::Key_W]) movement -= look * speed;
    if (m_keyMap[Qt::Key_S]) movement += look * speed;
    if (m_keyMap[Qt::Key_A]) movement -= right * speed;
    if (m_keyMap[Qt::Key_D]) movement += right * speed;
    if (m_keyMap[Qt::Key_Space]) movement += up * speed;
    if (m_keyMap[Qt::Key_Control]) movement -= up * speed;

    if (glm::length(movement) > 0.0f) {

        m_camera.translate(movement);
        update();

    }

}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int fixedWidth = viewport[2];
    int fixedHeight = viewport[3];

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
