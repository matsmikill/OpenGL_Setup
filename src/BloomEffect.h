#ifndef BLOOM_EFFECT_H
#define BLOOM_EFFECT_H

#include <glad/glad.h>
#include "Shader.h"

class BloomEffect {
public:
    BloomEffect(int width, int height);
    ~BloomEffect();

    void setupFramebuffers();
    void renderBloomTexture(unsigned int srcTexture, float threshold);
    void blurBloomTexture(int iterations);
    void renderFinalImage(unsigned int srcTexture, float exposure);

    unsigned int getHdrFBO() const { return m_hdrFBO; }
    unsigned int getColorBuffer(int index) const { return m_colorBuffers[index]; }

private:
    int m_width, m_height;
    unsigned int m_hdrFBO, m_colorBuffers[2];
    unsigned int m_pingpongFBO[2], m_pingpongColorBuffers[2];
    unsigned int m_quadVAO, m_quadVBO;

    Shader m_bloomShader;
    Shader m_blurShader;
    Shader m_finalShader;

    void renderQuad();
};

#endif // BLOOM_EFFECT_H