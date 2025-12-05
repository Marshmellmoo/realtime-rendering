#include "postprocess.h"
#include <iostream>

void PostProcessPipeline::initialize(int width, int height, GLuint fullscreenVAO) {
    m_width = width;
    m_height = height;
    m_fullscreenVAO = fullscreenVAO;
    
    createPingPongBuffers();
    m_initialized = true;
}

void PostProcessPipeline::createPingPongBuffers() {
    // Generate FBOs and textures for ping-pong rendering
    glGenFramebuffers(2, m_pingpongFBO);
    glGenTextures(2, m_pingpongTextures);
    
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
        
        // Create color texture
        glBindTexture(GL_TEXTURE_2D, m_pingpongTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Attach to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                              GL_TEXTURE_2D, m_pingpongTextures[i], 0);
        
        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Post-process ping-pong FBO " << i << " not complete!" << std::endl;
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PostProcessPipeline::addEffect(const PostProcessEffect& effect) {
    m_effects.push_back(effect);
}

void PostProcessPipeline::removeEffect(const std::string& name) {
    m_effects.erase(
        std::remove_if(m_effects.begin(), m_effects.end(),
            [&name](const PostProcessEffect& e) { return e.name == name; }),
        m_effects.end()
    );
}

void PostProcessPipeline::setEffectEnabled(const std::string& name, bool enabled) {
    for (auto& effect : m_effects) {
        if (effect.name == name) {
            effect.enabled = enabled;
            break;
        }
    }
}

void PostProcessPipeline::process(GLuint inputTexture, GLuint depthTexture,
                                  GLuint outputFBO, int width, int height) {
    if (!m_initialized || m_effects.empty()) {
        // No effects, just copy input to output
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
        glViewport(0, 0, width, height);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        
        // Simple copy would go here if needed
        return;
    }
    
    glDisable(GL_DEPTH_TEST);
    
    // Count enabled effects
    std::vector<PostProcessEffect*> activeEffects;
    for (auto& effect : m_effects) {
        if (effect.enabled) {
            activeEffects.push_back(&effect);
        }
    }
    
    if (activeEffects.empty()) {
        return;
    }
    
    // Process effects using ping-pong buffers
    GLuint currentInput = inputTexture;
    bool pingpong = true; // Start with first buffer
    
    for (size_t i = 0; i < activeEffects.size(); i++) {
        PostProcessEffect* effect = activeEffects[i];
        
        // Determine output target
        GLuint outputTarget;
        if (i == activeEffects.size() - 1) {
            // Last effect - render to output FBO
            outputTarget = outputFBO;
        } else {
            // Intermediate effect - render to ping-pong buffer
            outputTarget = m_pingpongFBO[pingpong ? 0 : 1];
        }
        
        // Bind output framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, outputTarget);
        glViewport(0, 0, width, height);
        
        // Setup blending if needed
        if (effect->useBlending) {
            glEnable(GL_BLEND);
            glBlendFunc(effect->blendSrc, effect->blendDst);
        } else {
            glDisable(GL_BLEND);
        }
        
        // Render the effect
        renderQuad(effect->shader, currentInput, depthTexture, width, height, *effect);
        
        // For next iteration, input becomes the current ping-pong texture
        if (i < activeEffects.size() - 1) {
            currentInput = m_pingpongTextures[pingpong ? 0 : 1];
            pingpong = !pingpong;
        }
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessPipeline::renderQuad(GLuint shader, GLuint texture, GLuint depthTexture,
                                     int width, int height, const PostProcessEffect& effect) {
    glUseProgram(shader);
    
    // Bind input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "inputTexture"), 0);
    
    // Bind depth texture if needed
    if (effect.needsDepth && depthTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glUniform1i(glGetUniformLocation(shader, "depthTexture"), 1);
    }
    
    // Call custom setup function if provided
    if (effect.setup) {
        effect.setup(shader, texture, width, height);
    }
    
    // Draw fullscreen quad
    glBindVertexArray(m_fullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PostProcessPipeline::resize(int width, int height) {
    if (width == m_width && height == m_height) {
        return;
    }
    
    m_width = width;
    m_height = height;
    
    // Recreate ping-pong buffers with new size
    if (m_initialized) {
        cleanup();
        createPingPongBuffers();
    }
}

size_t PostProcessPipeline::getActiveEffectCount() const {
    size_t count = 0;
    for (const auto& effect : m_effects) {
        if (effect.enabled) count++;
    }
    return count;
}

void PostProcessPipeline::clear() {
    m_effects.clear();
}

void PostProcessPipeline::cleanup() {
    if (m_pingpongFBO[0] != 0) {
        glDeleteFramebuffers(2, m_pingpongFBO);
        m_pingpongFBO[0] = m_pingpongFBO[1] = 0;
    }
    
    if (m_pingpongTextures[0] != 0) {
        glDeleteTextures(2, m_pingpongTextures);
        m_pingpongTextures[0] = m_pingpongTextures[1] = 0;
    }
}
