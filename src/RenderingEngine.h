#ifndef RENDERINGENGINE_H
#define RENDERINGENGINE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "components/PointLight.h"

struct GLFWwindow;
struct GLFWmonitor;
class FontRenderer;
class Camera;
class Transform;
class Time;
class Material;
class Cloth;
class RenderingEngine {
  public:
    RenderingEngine();
    ~RenderingEngine();

    bool initWindow(const std::string& title, int w, int h);
    void initVertex();
    bool initShader();
    bool isFullscreen();
    int render();
    void renderFont();
    void renderScene(unsigned int shader);
    void renderFrame();

    static RenderingEngine* GetInstance() { return instance; }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    void Invalidate();

  private:
    void mouseCallback(double xpos, double ypos);
    void keyboardCallback();

  private:
    static RenderingEngine* instance;

    unsigned int normal_shader, depth_cubemap_shader, shadow_cubemap_shader, cloth_shader;
    unsigned int cubeVAO, cubeVBO, planeVAO, planeVBO, dragonVAO, dragonVBO;
    unsigned int gpuTimeProfileQuery, timeElapsed;
    int width, height;
    double MouseSensitivity, lastMouseX, lastMouseY;
    bool hdrKeyPressed, useNormalKeyPressed;

    GLFWwindow* mWindow;
    GLFWmonitor* mMonitor;
    FontRenderer* fontRenderer;
    Camera* camera;
    Transform* cameraTrans, *cloth_transform;
    Time* time;
    Material *cube1_material, *cube2_material;
    Cloth* cloth;
    std::vector<PointLight*> lights;
    bool isInvalidate;
};

#endif  // RENDERINGENGINE_H