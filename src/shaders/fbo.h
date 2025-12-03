#pragma once
#include <GL/glew.h>

class FBO {

public:

    FBO(int width, int height);
    ~FBO();

    void bind();
    void unbind();

    GLuint getColorTexture() const { return m_colorTexture; }
    GLuint getDepthTexture() const { return m_depthTexture; }

private:

    GLuint m_fbo;
    GLuint m_colorTexture;
    GLuint m_depthTexture;
    int m_width, m_height;

};
