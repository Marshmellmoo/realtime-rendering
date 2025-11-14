#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
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
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    glClearColor(1.0, 0.0, 0.0, 1.0);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_phong = ShaderLoader::createShaderProgram(
        ":/resources/shaders/default.vert",
        ":/resources/shaders/default.frag"
    );

    initializeShapeGeometry();
    geometryInit = true;

}

// void insert(std::vector<float> &data, glm::vec3 v) {
//     data.push_back(v.x);
//     data.push_back(v.y);
//     data.push_back(v.z);
// }

// void setVertexData() {

//     glm::vec3 vecA = glm::vec3(-0.5f, 0.5f, 0.0f);
//     glm::vec3 vecB = glm::vec3(-0.5f, -0.5f, 0.0f);
//     glm::vec3 vecC = glm::vec3(0.5f, -0.5f, 0.0f);
//     glm::vec3 norm = glm::vec3(0, 0, 1);

//     std::vector<float> data;
//     GLuint vbo;
//     GLuint vao;

//     insert(data, vecA);
//     insert(data, norm);
//     insert(data, vecB);
//     insert(data, norm);
//     insert(data, vecC);
//     insert(data, norm);

//     glGenBuffers(1, &vbo);
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
//     glBufferData(GL_ARRAY_BUFFER,
//                  data.size() * sizeof(GLfloat),
//                  data.data(),
//                  GL_STATIC_DRAW);

//     glGenVertexArrays(1, &vao);
//     glBindVertexArray(vao);

//     // Positions
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
//                           6 * sizeof(float),
//                           (void*)0);

//     // Normals
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
//                           6 * sizeof(float),
//                           (void*)(3 * sizeof(float)));

//     glBindVertexArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);

// }

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
                          6 * sizeof(float),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

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
                          6 * sizeof(float),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

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
                          6 * sizeof(float),
                          (void*)0);

    // Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(float),
                          (void*)(3 * sizeof(float)));

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_phong);

    glUniformMatrix4fv(glGetUniformLocation(m_phong, "view"),
                       1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_phong, "proj"),
                       1, GL_FALSE, &m_projection[0][0]);

    glm::vec3 camPosition = m_camera.getInverseViewMatrix()[3];
    glUniform3fv(glGetUniformLocation(m_phong, "cameraPosition"),
                 1, &camPosition[0]);

    glUniform1f(glGetUniformLocation(m_phong, "ka"),
                m_global.ka);
    glUniform1f(glGetUniformLocation(m_phong, "kd"),
                m_global.kd);
    glUniform1f(glGetUniformLocation(m_phong, "ks"),
                m_global.ks);


    passLightsToShader(m_phong);

    for (const auto& shape : m_renderData.shapes) {
        drawShape(shape);
    }

    // setVertexData();

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

void Realtime::drawShape(const RenderShapeData& shape) {

    glUniformMatrix4fv(glGetUniformLocation(m_phong, "model"),
                       1, GL_FALSE, &shape.ctm[0][0]);

    auto info = shape.primitive.material;

    glm::vec4 ambient = info.cAmbient;
    glm::vec4 diffuse = info.cDiffuse;
    glm::vec4 specular = info.cSpecular;


    glUniform4fv(glGetUniformLocation(m_phong, "c_ambient"),
                 1, &ambient[0]);
    glUniform4fv(glGetUniformLocation(m_phong, "c_diffuse"),
                 1, &diffuse[0]);
    glUniform4fv(glGetUniformLocation(m_phong, "c_specular"),
                 1, &specular[0]);

    glUniform1f(glGetUniformLocation(m_phong, "shininess"),
                info.shininess);

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

    std::cout << std::to_string(verticies) << std::endl;

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, verticies);
    glBindVertexArray(0);

}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    float aspectRatio = (float)w / (float)h;
    m_camera.setAspectRatio(aspectRatio);

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
        std::cerr << "Failed to parse scene file" << std::endl;
        return;
    }

    // Debug output
    std::cout << "Scene loaded successfully!" << std::endl;
    std::cout << "  Shapes: " << m_renderData.shapes.size() << std::endl;
    std::cout << "  Lights: " << m_renderData.lights.size() << std::endl;
    std::cout << "  Camera pos: (" << m_renderData.cameraData.pos.x << ", "
              << m_renderData.cameraData.pos.y << ", "
              << m_renderData.cameraData.pos.z << ")" << std::endl;

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

    initializeShapeGeometry();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {

    m_camera.setNearPlane(settings.nearPlane);
    m_camera.setFarPlane(settings.farPlane);

    m_view = m_camera.getViewMatrix();
    m_projection = m_camera.getProjectionMatrix();

    if (geometryInit) initializeShapeGeometry();

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

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

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
