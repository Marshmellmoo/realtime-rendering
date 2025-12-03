#include "fbo.h"
#include <iostream>

FBO::FBO(int width, int height) : m_width(width), m_height(height) {

    // glGenFramebuffers(1, &m_fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // // Color Texture Assignment --
    // glGenTextures(1, &m_colorTexture);
    // glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
    //              GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    //                        GL_TEXTURE_2D, m_colorTexture, 0);

    // // Depth Texture Assignment --
    // glGenTextures(1, &m_depthTexture);
    // glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
    //              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
    //                        GL_TEXTURE_2D, m_depthTexture, 0);


    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // std::cout << "  FBO status: 0x" << std::hex << status << std::dec << std::endl;

    // if (status != GL_FRAMEBUFFER_COMPLETE) {
    //     std::cerr << "ERROR: FBO INCOMPLETE!" << std::endl;

    //     switch(status) {
    //     case GL_FRAMEBUFFER_UNDEFINED:
    //         std::cerr << "  - GL_FRAMEBUFFER_UNDEFINED" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
    //         std::cerr << "    (One of the attachments is incomplete)" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
    //         std::cerr << "    (No attachments)" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_UNSUPPORTED:
    //         std::cerr << "  - GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
    //         std::cerr << "    (Format combination not supported)" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
    //         break;
    //     case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    //         std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
    //         break;
    //     default:
    //         std::cerr << "  - Unknown error: 0x" << std::hex << status << std::dec << std::endl;
    //     }
    // } else {
    //     std::cout << "  FBO is COMPLETE!" << std::endl;
    // }

    std::cout << "\n=== Creating FBO ===" << std::endl;
    std::cout << "Requested size: " << width << "x" << height << std::endl;

    // Check if we have a valid OpenGL context
    GLint currentFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    std::cout << "Current FBO before creation: " << currentFBO << std::endl;

    // Check max texture size
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    std::cout << "Max texture size: " << maxSize << std::endl;

    if (width > maxSize || height > maxSize) {
        std::cerr << "ERROR: Requested size exceeds max texture size!" << std::endl;
        return;
    }

    // Generate framebuffer
    glGenFramebuffers(1, &m_fbo);
    std::cout << "Generated FBO ID: " << m_fbo << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Verify it's bound
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    std::cout << "FBO bound, current binding: " << currentFBO << std::endl;

    // ========== Color Texture ==========
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach color texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_colorTexture, 0);

    std::cout << "Color texture created: " << m_colorTexture << std::endl;

    // Check color attachment
    GLint colorAttachment;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                                          &colorAttachment);
    std::cout << "Color attachment verified: " << colorAttachment << std::endl;

    // ========== Depth Renderbuffer (more compatible than texture) ==========
    glGenRenderbuffers(1, &m_depthTexture);  // Reusing m_depthTexture variable for RBO
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthTexture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthTexture);

    std::cout << "Depth renderbuffer created: " << m_depthTexture << std::endl;

    // ========== Check Completeness ==========
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    std::cout << "FBO status: 0x" << std::hex << status << std::dec;

    if (status == GL_FRAMEBUFFER_COMPLETE) {
        std::cout << " (GL_FRAMEBUFFER_COMPLETE)" << std::endl;
        std::cout << "✅ FBO created successfully!" << std::endl;
    } else {
        std::cout << std::endl;
        std::cerr << "❌ ERROR: FBO INCOMPLETE!" << std::endl;

        switch(status) {
        case 0x8219:  // GL_FRAMEBUFFER_UNDEFINED
            std::cerr << "  - GL_FRAMEBUFFER_UNDEFINED" << std::endl;
            std::cerr << "    OpenGL context is not active!" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            std::cerr << "  - GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            std::cerr << "  - GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
            break;
        default:
            std::cerr << "  - Unknown error" << std::endl;
        }
    }

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "=== FBO Creation Complete ===\n" << std::endl;

}

FBO::~FBO() {

    glDeleteTextures(1, &m_colorTexture);
    glDeleteRenderbuffers(1, &m_depthTexture);
    glDeleteFramebuffers(1, &m_fbo);
}

void FBO::bind() {

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

}

void FBO::unbind() {

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
