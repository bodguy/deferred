#ifndef DEFERRED_RENDERINGENGINE_H
#define DEFERRED_RENDERINGENGINE_H

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct PointLight {
  PointLight(glm::vec3 p, glm::vec3 c)
    : position(p), color(c), attenuation(0.1f),
    shadowBias(0.01), shadowFilterSharpen(0.01), shadowStrength(1.f), nearPlane(0.1f), intensity(1.f),
    castShadow(true), castTranslucentShadow(true), shadowMapResolution(glm::vec2(512.f, 512.f)) {}

  glm::vec3 position;
  glm::vec3 color;
  float attenuation;
  float shadowBias;
  float shadowFilterSharpen;
  float shadowStrength;
  float nearPlane;
  float intensity;
  bool castShadow;
  bool castTranslucentShadow;
  glm::vec2 shadowMapResolution;
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
  float far_plane;
  float deltaTime, lastFrame, Yaw, Pitch, MouseSensitivity, lastX, lastY;
  bool firstMouse;
  unsigned int depth_shader, shadow_shader, depth_visual_shader, normal_shader, depth_cubemap_shader, shadow_cubemap_shader;
  unsigned int cubeVAO, cubeVBO, quadVAO, quadVBO, planeVAO, planeVBO;
  int width, height;
  unsigned int diffuse_texture, diffuse_texture2, normal_texture;
  unsigned int depthCubeMapFBO[4], depthCubeMap[4], depthMapFBO, depthMap;
  unsigned int gpuTimeProfileQuery;
  unsigned int timeElapsed;
  std::vector<PointLight> lights;
  FontRenderer* fontRenderer;
};


#endif //DEFERRED_RENDERINGENGINE_H
