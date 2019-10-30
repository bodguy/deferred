#ifndef DEFERRED_RENDERINGENGINE_H
#define DEFERRED_RENDERINGENGINE_H

#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct Character {
  unsigned int TextureID;   // ID handle of the glyph texture
  unsigned char letter;
  glm::ivec2 Size;    // Size of glyph
  glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
  signed long Advance;    // Horizontal offset to advance to next glyph

  void printInfo() {
    std::cout << "letter: " << (char) letter << " (ASCII: " << (unsigned int) letter << ")" << std::endl;
    std::cout << "Size.x: " << Size.x << ", Size.y: " << Size.y << std::endl;
    std::cout << "Bearing.x: " << Bearing.x << ", Bearing.y: " << Bearing.y << std::endl;
    std::cout << "Advance: " << Advance << std::endl;
  }
};

struct GLFWwindow;

class RenderingEngine {
public:
  RenderingEngine();
  ~RenderingEngine();

  bool initWindow(const std::string &title, int w, int h);
  void initVertex();
  bool initFramebuffer();
  bool initFont(const std::string &filename);
  bool initShader();
  bool initTexture();
  int render();
  void renderScene(unsigned int shader);
  void renderQuad();
  void renderFrame();
  void renderLight();
  void text(std::string text, glm::vec2 pos, glm::vec3 color);

private:
  void mouseCallback(double xpos, double ypos);
  void keyboardCallback();
  void updateDeltaTime();
  void updateCamera();

private:
  static RenderingEngine *instance;

  GLFWwindow *mWindow;
  glm::vec3 cameraPos, cameraFront, cameraUp, cameraRight;
  glm::vec3 lightPos;
  glm::mat4 projection, view;
  float deltaTime, lastFrame, Yaw, Pitch, MouseSensitivity, lastX, lastY;
  bool firstMouse;
  unsigned int font_shader, depth_shader, shadow_shader, depth_visual_shader, normal_shader;
  unsigned int fontVAO, fontVBO;
  unsigned int cubeVAO, cubeVBO;
  unsigned int quadVAO, quadVBO;
  unsigned int planeVAO, planeVBO;
  int width, height;
  unsigned int wood_texture;
  unsigned int depthMapFBO, depthMap;
  std::map<char, Character> mCharMap;
};


#endif //DEFERRED_RENDERINGENGINE_H
