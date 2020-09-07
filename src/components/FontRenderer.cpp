#include "FontRenderer.h"

#include <GL/glew.h>
#include <ft2build.h>

#include <cstdarg>
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "../RenderingEngine.h"
#include "../util.h"
#include FT_FREETYPE_H

FontRenderer::FontRenderer() : mScale(0.5f), mColor(glm::vec3(1.f)), mFontShader(0), mFontVAO(0), mFontVBO(0) { mCharMap.clear(); }

FontRenderer::~FontRenderer() {
    for (auto& i : mCharMap) {
        glDeleteTextures(1, &i.second.TextureID);
    }
    mCharMap.clear();

    glDeleteVertexArrays(1, &mFontVAO);
    glDeleteBuffers(1, &mFontVBO);
    glDeleteProgram(mFontShader);
}

bool FontRenderer::Init(const std::string& filename) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, filename.c_str(), 0, &face)) {
        std::cout << "Failed to load font" << std::endl;
        return false;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++) {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint font_texture;
        glGenTextures(1, &font_texture);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {font_texture, c, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows), glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), face->glyph->advance.x};
        mCharMap.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glGenVertexArrays(1, &mFontVAO);
    glGenBuffers(1, &mFontVBO);
    glBindVertexArray(mFontVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mFontVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mFontShader = loadShaderFromFile("../shaders/font/font_vs.shader", "../shaders/font/font_fs.shader");
    if (!mFontShader) return false;

    return true;
}

void FontRenderer::Print(const std::string& text, const glm::vec2& pos) { DrawText(text, pos); }

void FontRenderer::Printf(const glm::vec2& pos, const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    DrawText(buffer, pos);  // implicit conversion
}

std::string FontRenderer::Sprintf(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    std::string ret(buffer);
    return ret;
}

void FontRenderer::SetScale(float scale) { mScale = scale; }

void FontRenderer::SetColor(const glm::vec3 color) { mColor = color; }

void FontRenderer::PrintCodepoint(const char codepoint) {
    std::map<char, Character>::iterator found = mCharMap.find(codepoint);
    if (found == mCharMap.end()) {
        return;
    }
    Character ch = found->second;
    std::cout << "letter: " << static_cast<char>(codepoint) << " (ASCII: " << static_cast<unsigned int>(codepoint) << ")" << std::endl;
    std::cout << "Size.x: " << ch.Size.x << ", Size.y: " << ch.Size.y << std::endl;
    std::cout << "Bearing.x: " << ch.Bearing.x << ", Bearing.y: " << ch.Bearing.y << std::endl;
    std::cout << "Advance: " << ch.Advance << std::endl;
}

void FontRenderer::DrawText(std::string text, glm::vec2 pos) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(mFontShader);
    glUniform3f(glGetUniformLocation(mFontShader, "textColor"), mColor.x, mColor.y, mColor.z);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(RenderingEngine::GetInstance()->GetWidth()), 0.0f, static_cast<GLfloat>(RenderingEngine::GetInstance()->GetHeight()));
    glUniformMatrix4fv(glGetUniformLocation(mFontShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mFontVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = mCharMap[*c];

        float xpos = pos.x + ch.Bearing.x * mScale;
        float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * mScale;

        float w = ch.Size.x * mScale;
        float h = ch.Size.y * mScale;
        // Update VBO for each character
        float vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},     {xpos + w, ypos, 1.0, 1.0},

                                {xpos, ypos + h, 0.0, 0.0}, {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, mFontVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        pos.x += (ch.Advance >> 6) * mScale;  // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}