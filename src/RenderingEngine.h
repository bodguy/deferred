#ifndef DEFERRED_RENDERINGENGINE_H
#define DEFERRED_RENDERINGENGINE_H

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct PointLight {
  PointLight(glm::vec3 p, glm::vec3 a)
    : position(p), constant(1.0f), linear(0.09f), quadratic(0.032f), shadow_bias(0.01), ambient(a * 0.8f), diffuse(a), specular(a) {}

  glm::vec3 position;
  float constant;
  float linear;
  float quadratic;
  float shadow_bias;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
};

struct GLFWwindow;
class FontRenderer;
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
  void renderLight();

  static RenderingEngine* GetInstance() { return instance; }

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

private:
  void mouseCallback(double xpos, double ypos);
  void keyboardCallback();
  void updateDeltaTime();
  void updateCamera();

private:
  static RenderingEngine *instance;

  GLFWwindow *mWindow;
  glm::vec3 cameraPos, cameraFront, cameraUp, cameraRight;
  glm::mat4 projection, view;
  std::vector<glm::vec3> movablePointLights;
  float near_plane, far_plane;
  float deltaTime, lastFrame, Yaw, Pitch, MouseSensitivity, lastX, lastY;
  bool firstMouse;
  unsigned int depth_shader, shadow_shader, depth_visual_shader, normal_shader, depth_cubemap_shader, shadow_cubemap_shader;
  unsigned int cubeVAO, cubeVBO, quadVAO, quadVBO, planeVAO, planeVBO;
  int width, height;
  unsigned int diffuse_texture, diffuse_texture2;
  unsigned int depthCubeMapFBO[4], depthCubeMap[4], depthMapFBO, depthMap;
  bool usePcf, usePcfKeyPress, useShadow, shadowKeyPress;
  float shadowMapWidth, shadowMapHeight;
  std::vector<PointLight> lights;
  FontRenderer* fontRenderer;
};


#endif //DEFERRED_RENDERINGENGINE_H
