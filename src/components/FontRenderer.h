#ifndef FONTRENDERER_H
#define FONTRENDERER_H

#include <map>
#include <string>
#include <glm/glm.hpp>

struct Character {
  unsigned int TextureID;   // ID handle of the glyph texture
  unsigned char letter;
  glm::ivec2 Size;    // Size of glyph
  glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
  signed long Advance;    // Horizontal offset to advance to next glyph
};

class FontRenderer {
public:
  FontRenderer();
  ~FontRenderer();

  bool Init(const std::string& filename);

  void Print(const std::string& text, const glm::vec2& pos);
  void Printf(const glm::vec2& pos, const char* fmt, ...);
  std::string Sprintf(const char* fmt, ...);

  void SetColor(const glm::vec3 color);
  void PrintCodepoint(const char codepoint);
  unsigned long GetGlyphSize() const { return mCharMap.size(); }

private:
  void DrawText(std::string text, glm::vec2 pos);

private:
  glm::vec3 mColor;
  std::map<char, Character> mCharMap;
  unsigned int mFontShader;
  unsigned int mFontVAO, mFontVBO;
};


#endif // FONTRENDERER_H
