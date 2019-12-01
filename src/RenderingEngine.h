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
class Material;
class RenderingEngine {
public:
  RenderingEngine();
  ~RenderingEngine();

  bool initWindow(const std::string &title, int w, int h);
  void initVertex();
  bool initShader();
  int render();
  void renderFont();
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

  unsigned int normal_shader, depth_cubemap_shader, shadow_cubemap_shader;
  unsigned int cubeVAO, cubeVBO, planeVAO, planeVBO;
  unsigned int gpuTimeProfileQuery, timeElapsed;
  int width, height;
  double MouseSensitivity, lastMouseX, lastMouseY;
  bool hdrKeyPressed;

  GLFWwindow *mWindow;
  FontRenderer* fontRenderer;
  Camera* camera;
  Transform* cameraTrans;
  Time* time;
  Material *cube1, *cube2;
  std::vector<PointLight*> lights;
};


#endif // RENDERINGENGINE_H