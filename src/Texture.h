#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    void bind(unsigned int slot = 0) const;

    unsigned int getID() const { return m_RendererID; }

private:
    unsigned int m_RendererID;
    std::string m_FilePath;
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_BPP;
};

#endif // TEXTURE_H
