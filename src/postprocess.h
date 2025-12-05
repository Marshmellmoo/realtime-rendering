#pragma once

#include <GL/glew.h>
#include <vector>
#include <functional>
#include <string>

/**
 * @brief Post-processing effect structure
 * 
 * Each effect consists of:
 * - A shader program
 * - A setup function to configure uniforms/state
 * - Optional blend mode settings
 */
struct PostProcessEffect {
    std::string name;
    GLuint shader;
    
    // Function to setup uniforms and state before rendering
    // Parameters: shader program, source texture, screen width, screen height
    std::function<void(GLuint, GLuint, int, int)> setup;
    
    // Optional: custom blend mode
    bool useBlending = false;
    GLenum blendSrc = GL_SRC_ALPHA;
    GLenum blendDst = GL_ONE;
    
    // Optional: requires depth texture
    bool needsDepth = false;
    
    // Optional: whether effect is enabled
    bool enabled = true;
    
    PostProcessEffect(const std::string& effectName, GLuint shaderProgram)
        : name(effectName), shader(shaderProgram) {}
};

/**
 * @brief Post-processing Pipeline Manager
 * 
 *  Manages a chain of post-processing effects applied sequentially.
 *  Uses ping-pong buffers for multi-pass effects.
 */

class PostProcessPipeline {
public:
    PostProcessPipeline() = default;
    ~PostProcessPipeline() = default;
    
    void initialize(int width, int height, GLuint fullscreenVAO);
    
    /**
     * @brief Add an effect to the pipeline.
     */
    void addEffect(const PostProcessEffect& effect);
    
    /**
     * @brief Remove an effect by name
     */
    void removeEffect(const std::string& name);
    
    /**
     * @brief Enable/disable an effect by name
     */
    void setEffectEnabled(const std::string& name, bool enabled);
    
    /**
     * @brief Process the input texture through the entire pipeline
     * 
     * @param inputTexture Source texture (scene render)
     * @param depthTexture Optional depth texture (if any effect needs it)
     * @param outputFBO Target framebuffer (usually default FBO)
     * @param width Screen width
     * @param height Screen height
     */
    void process(GLuint inputTexture, GLuint depthTexture, 
                GLuint outputFBO, int width, int height);
    
    /**
     * @brief Resize internal buffers when window size changes
     */
    void resize(int width, int height);
    
    /**
     * @brief Get number of active effects
     */
    size_t getActiveEffectCount() const;
    
    /**
     * @brief Clear all effects
     */
    void clear();
    
    /**
     * @brief Cleanup OpenGL resources
     */
    void cleanup();
    
private:
    std::vector<PostProcessEffect> m_effects;
    GLuint m_fullscreenVAO = 0;
    
    // Ping-pong FBOs for multi-pass rendering
    GLuint m_pingpongFBO[2] = {0, 0};
    GLuint m_pingpongTextures[2] = {0, 0};
    
    int m_width = 0;
    int m_height = 0;
    bool m_initialized = false;
    
    // Helper to create ping-pong buffers
    void createPingPongBuffers();
    
    // Helper to render a fullscreen quad with a texture
    void renderQuad(GLuint shader, GLuint texture, GLuint depthTexture, 
                   int width, int height, const PostProcessEffect& effect);
};
