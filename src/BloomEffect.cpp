#include "BloomEffect.h"

BloomEffect::BloomEffect(int width, int height)
    : m_width(width), m_height(height),
    m_bloomShader("Shaders/bloom.vert", "Shaders/bloom.frag"),
    m_blurShader("Shaders/blur.vert", "Shaders/blur.frag"),
    m_finalShader("Shaders/final.vert", "Shaders/final.frag") {
    setupFramebuffers();
}

BloomEffect::~BloomEffect() {
    glDeleteFramebuffers(1, &m_hdrFBO);
    glDeleteFramebuffers(2, m_pingpongFBO);
    glDeleteTextures(2, m_colorBuffers);
    glDeleteTextures(2, m_pingpongColorBuffers);
    glDeleteVertexArrays(1, &m_quadVAO);
    glDeleteBuffers(1, &m_quadVBO);
}

void BloomEffect::setupFramebuffers() {
    // Setup for HDR framebuffer
    glGenFramebuffers(1, &m_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    glGenTextures(2, m_colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorBuffers[i], 0);
    }

    // Setup for ping-pong framebuffers
    glGenFramebuffers(2, m_pingpongFBO);
    glGenTextures(2, m_pingpongColorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, m_pingpongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongColorBuffers[i], 0);
    }
}

void BloomEffect::renderBloomTexture(unsigned int srcTexture, float threshold) {
    m_bloomShader.use();
    m_bloomShader.setFloat("threshold", threshold);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    renderQuad();
}

void BloomEffect::blurBloomTexture(int iterations) {
    m_blurShader.use();
    for (unsigned int i = 0; i < iterations; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i % 2]);
        m_blurShader.setInt("horizontal", i % 2);
        glBindTexture(GL_TEXTURE_2D, i == 0 ? m_colorBuffers[1] : m_pingpongColorBuffers[(i - 1) % 2]);
        renderQuad();
    }
}

void BloomEffect::renderFinalImage(unsigned int srcTexture, float exposure) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_finalShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_pingpongColorBuffers[1]);
    m_finalShader.setInt("scene", 0);
    m_finalShader.setInt("bloomBlur", 1);
    m_finalShader.setFloat("exposure", exposure);
    renderQuad();
}

void BloomEffect::renderQuad() {
    if (m_quadVAO == 0) {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}