#ifndef RENDERINGENGINE_H
#define RENDERINGENGINE_H

#include "components/PointLight.h"
#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct GLFWwindow;
class FontRenderer;
class Camera;
class Transform;
class Time;
class RenderingEngine {
public:
  RenderingEngine();
  ~RenderingEngine();

  bool initWindow(const std::string &title, int w, int h);
  void initVertex();
  bool initFramebuffer();
  bool initShader();
  bool initTexture();
  int render();
  void renderScene(unsigned int shader);
  void renderFrame();

  static RenderingEngine* GetInstance() { return instance; }

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }
  void SetSize(int w, int h);

private:
  void mouseCallback(double xpos, double ypos);
  void keyboardCallback();

private:
  static RenderingEngine *instance;

  GLFWwindow *mWindow;
  float MouseSensitivity, lastX, lastY;
  bool firstMouse;
  unsigned int depth_shader, shadow_shader, depth_visual_shader, normal_shader, depth_cubemap_shader, shadow_cubemap_shader;
  unsigned int cubeVAO, cubeVBO, planeVAO, planeVBO;
  int width, height;
  unsigned int diffuse_texture, diffuse_texture2, normal_texture;
  unsigned int depthMapFBO, depthMap;
  unsigned int gpuTimeProfileQuery;
  unsigned int timeElapsed;
  bool hdrKeyPressed;
  float x_offset, y_offset;
  std::vector<PointLight*> lights;
  FontRenderer* fontRenderer;
  Camera* camera;
  Transform* trans;
  Time* time;
};


#endif // RENDERINGENGINE_H