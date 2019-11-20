#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stb_image.h>

namespace utils {
  const extern float planeVertices[48];
  const extern float cubeVertices[288];
  const extern float quadVertices[24];
}

extern int drawCallCount;
extern int vertexCount;
extern int triangleCount;

bool loadFile(const std::string& filepath, std::string& out_source);
unsigned int loadShaderFromFile(const std::string& vs_name, const std::string& fs_name);
unsigned int loadShaderFromFile(const std::string& vs_name, const std::string& gs_name, const std::string& fs_name);
unsigned int loadTexture(char const * path, bool useSRGB);
void glDrawArrays_profile(GLenum mode, GLint first, GLsizei count);
void resetProfile();

template <typename T>
struct Rect {
  Rect() :x(0), y(0), w(0), h(0) {}
  Rect(T _x, T _y, T _w, T _h) :x(_x), y(_y), w(_w), h(_h) {}
  T x, y, w, h;
};

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#endif // UTIL_H