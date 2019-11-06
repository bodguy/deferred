#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <stb_image.h>

namespace utils {
  const extern glm::vec3 up, down, left, right, forward, backward, one, zero;
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

#endif // UTIL_H