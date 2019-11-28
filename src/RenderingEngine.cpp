#include "RenderingEngine.h"
#include "util.h"
#include "components/FontRenderer.h"
#include "components/Transform.h"
#include "components/Camera.h"
#include "components/Time.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

static void updateViewport(GLFWwindow* win, int w, int h) {
  RenderingEngine::GetInstance()->SetSize(w, h);
  // framebuffer resize needed
}

RenderingEngine *RenderingEngine::instance = nullptr;

RenderingEngine::RenderingEngine()
        : mWindow(nullptr),
          MouseSensitivity(0.3f), lastX(0.f), lastY(0.f),
          firstMouse(true),
          depth_shader(0), shadow_shader(0), depth_visual_shader(0), normal_shader(0), depth_cubemap_shader(0), shadow_cubemap_shader(0),
          cubeVAO(0), cubeVBO(0), planeVAO(0), planeVBO(0),
          width(0), height(0),
          diffuse_texture(0), diffuse_texture2(0), normal_texture(0),
          depthCubeMapFBO{0,}, depthCubeMap{0,}, depthMapFBO(0), depthMap(0),
          gpuTimeProfileQuery(0), timeElapsed(0), hdrKeyPressed(false), x_offset(0.f), y_offset(0.f), trans(nullptr) {
  instance = this;
  movablePointLights.clear();
  lights = {
    PointLight(glm::vec3( 3.17f,  2.34f,  -4.184f), glm::vec3(1.f, 1.f, 1.f)),
    PointLight(glm::vec3( 2.3f, 2.f, -4.0f),  glm::vec3(1.f, 1.f, 1.f)),
    PointLight(glm::vec3(2.3f, 2.f, -8.0f), glm::vec3(1.f, 1.f, 1.f)),
  };
  fontRenderer = new FontRenderer();
  camera = new Camera(glm::vec3(0.0f, 2.3f, 8.0f));
  time = new Time();
}

RenderingEngine::~RenderingEngine() {
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteBuffers(1, &planeVBO);
  glDeleteFramebuffers(lights.size(), depthCubeMapFBO);
  glDeleteTextures(lights.size(), depthCubeMap);
  glDeleteFramebuffers(1, &depthMapFBO);
  glDeleteTextures(1, &depthMap);

//  glDeleteProgram(depth_shader);
//  glDeleteProgram(shadow_shader);
//  glDeleteProgram(depth_visual_shader);
  glDeleteProgram(normal_shader);
  glDeleteProgram(depth_cubemap_shader);
  glDeleteProgram(shadow_cubemap_shader);
  glDeleteTextures(1, &diffuse_texture);
  glDeleteTextures(1, &diffuse_texture2);
  glDeleteTextures(1, &normal_texture);
  glDeleteQueries(1, &gpuTimeProfileQuery);
  SAFE_DEALLOC(fontRenderer);
  SAFE_DEALLOC(camera);
  SAFE_DEALLOC(time);
}

bool RenderingEngine::initWindow(const std::string &title, int w, int h) {
  if (!glfwInit()) {
    std::cout << "glfw init error" << std::endl;
    return false;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  mWindow = glfwCreateWindow(w, h, title.c_str(), NULL, NULL);
  if (!mWindow) {
    std::cout << "glfw window init error" << std::endl;
    glfwTerminate();
    return false;
  }
  glfwMakeContextCurrent(mWindow);
  glfwSetFramebufferSizeCallback(mWindow, updateViewport);
  glfwSetCursorPosCallback(mWindow, [](GLFWwindow *w, double x, double y) {
    instance->mouseCallback(x, y);
  });
  glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwPollEvents();

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cout << "glew init error" << std::endl;
    return false;
  }

  lastX = (float) w / 2.f;
  lastY = (float) h / 2.f;
  width = w;
  height = h;

  if (!fontRenderer->Init("../res/arial.ttf")) {
    return false;
  }

  if (!camera->Init()) {
    std::cout << "camera Init failed" << std::endl;
    return false;
  }
  trans = camera->GetTransform();

  glGenQueries(1, &gpuTimeProfileQuery);

  return true;
}

void RenderingEngine::initVertex() {
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(utils::planeVertices), utils::planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glBindVertexArray(0);

  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(utils::cubeVertices), &utils::cubeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
}

bool RenderingEngine::initFramebuffer() {
  // make depth framebuffer
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  glGenFramebuffers(1, &depthMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // make depth cubemap framebuffer
  glGenTextures(lights.size(), depthCubeMap);
  glGenFramebuffers(lights.size(), depthCubeMapFBO);
  for (int lc = 0; lc < lights.size(); lc++) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap[lc]);
    for (int i = 0; i < 6; ++i) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, lights[lc].shadowMapResolution.x, lights[lc].shadowMapResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, *(depthCubeMapFBO + lc));
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *(depthCubeMap + lc), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  return true;
}

bool RenderingEngine::initShader() {
  // for directional light shadow mapping (currently not used)
//  depth_shader = loadShaderFromFile("../shaders/shadow/depth_vs.shader", "../shaders/shadow/depth_fs.shader");
//  if (!depth_shader) return false;
//  shadow_shader = loadShaderFromFile("../shaders/shadow/shadow_vs.shader", "../shaders/shadow/shadow_fs.shader");
//  if (!shadow_shader) return false;
//  depth_visual_shader = loadShaderFromFile("../shaders/shadow/depth_visual_vs.shader", "../shaders/shadow/depth_visual_fs.shader");
//  if (!depth_visual_shader) return false;
  // for rendering light objects
  normal_shader = loadShaderFromFile("../shaders/normal/normal_vs.shader", "../shaders/normal/normal_fs.shader");
  if (!normal_shader) return false;
  // for point(omnidirecitonal) light shadow mapping
  depth_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/depth_vs.shader", "../shaders/point_shadow/depth_gs.shader", "../shaders/point_shadow/depth_fs.shader");
  if (!depth_cubemap_shader) return false;
  shadow_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/shadow_vs.shader", "../shaders/point_shadow/shadow_fs.shader");
  if (!shadow_cubemap_shader) return false;
  return true;
}

bool RenderingEngine::initTexture() {
  diffuse_texture = loadTexture("../res/wood.png", false);
  if (!diffuse_texture) return false;
  diffuse_texture2 = loadTexture("../res/brickwall.jpg", false);
  if (!diffuse_texture2) return false;
  normal_texture = loadTexture("../res/brickwall_normal.jpg", false);
  if (!normal_texture) return false;
  return true;
}

int RenderingEngine::render() {
  while (!glfwWindowShouldClose(mWindow)) {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(mWindow, true);

    time->Update();
    keyboardCallback();

    for (int i = 0; i < lights.size(); i++) {
      movablePointLights.push_back(glm::vec3(
              lights[i].position.x + cos(time->ElapsedTime() * (0.5f * (i + 1))) * 5.f,
              lights[i].position.y,
              lights[i].position.z + sin(time->ElapsedTime() * (0.5f * (i + 1))) * 5.f
      ));
    }

    glBeginQuery(GL_TIME_ELAPSED, gpuTimeProfileQuery);
    renderFrame();
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(gpuTimeProfileQuery, GL_QUERY_RESULT, &timeElapsed);

    movablePointLights.clear();

    fontRenderer->SetScale(0.27);
    fontRenderer->SetColor(glm::vec3(0.25f, 0.25f , 0.25f));
    fontRenderer->Printf(glm::vec2(5.f, height - 12 * 1), "triangle count: %d", triangleCount);
    fontRenderer->Printf(glm::vec2(5.f, height - 12 * 2), "vertex count: %d", vertexCount);
    fontRenderer->Printf(glm::vec2(5.f, height - 12 * 3), "draw call: %d", drawCallCount);
    fontRenderer->Printf(glm::vec2(5.f, height - 12 * 4), "GPU time: %d ns", timeElapsed);
    fontRenderer->SetScale(0.4);
    fontRenderer->SetColor(glm::vec3(1.f, 1.f , 1.f));
    fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 4), "use hdr: %s", camera->IsHdr() ? "true" : "false");
    fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 3), "exposure: %.5f", camera->GetHdrExposure());
    fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 2), "total lights: %ld", lights.size());
    glm::vec3 f = trans->GetForward();
    fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 1), "camera front: [%.2f, %.2f, %.2f]", f.x, f.y, f.z);
    glm::vec3 p = trans->GetPosition();
    fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 0), "camera pos: [%.2f, %.2f, %.2f]", p.x, p.y, p.z);

    resetProfile();
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void RenderingEngine::renderScene(unsigned int shader) {
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  // floor
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.84f, 2.12f, -4.52f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glBindVertexArray(planeVAO);
  glDrawArrays_profile(GL_TRIANGLES, 0, 6);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // first cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.92f, 2.12f, -6.51f));
  model = glm::scale(model, glm::vec3(0.5f));
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // another cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-0.36f, 2.46f, -4.66f));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture2);
  // cube1
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.92f, 2.12f, -2.35f));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // cube2
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(6.41f, 2.46f, -3.52f));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
}

void RenderingEngine::renderFrame() {
  std::vector<glm::mat4> shadowTransforms;
  for (int i = 0; i < lights.size(); i++) {
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), lights[i].shadowMapResolution.x / lights[i].shadowMapResolution.y, lights[i].nearPlane, camera->GetFarClipPlane());
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Right, Transform::Down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Left, Transform::Down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Up, Transform::Backward));
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Down, Transform::Forward));
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Backward, Transform::Down));
    shadowTransforms.push_back(shadowProj * glm::lookAt(movablePointLights[i], movablePointLights[i] + Transform::Forward, Transform::Down));
  }

  glEnable(GL_DEPTH_TEST);
  // 1. drawing geometry to depth cube map
  for (int lc = 0; lc < lights.size(); lc++) {
    glViewport(0, 0, lights[lc].shadowMapResolution.x, lights[lc].shadowMapResolution.y);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO[lc]);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(depth_cubemap_shader);
    for (int i = 0; i < 6; ++i) {
      glUniformMatrix4fv(glGetUniformLocation(depth_cubemap_shader, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i + lc * 6]));
    }
    glUniform1f(glGetUniformLocation(depth_cubemap_shader, "far_plane"), camera->GetFarClipPlane());
    glUniform3fv(glGetUniformLocation(depth_cubemap_shader, "lightPos"), 1, glm::value_ptr(movablePointLights[lc]));
    renderScene(depth_cubemap_shader);
  }

  // 2. drawing to the hdr floating point framebuffer
  glViewport(0, 0, width, height);
//  glBindFramebuffer(GL_FRAMEBUFFER, camera->GetHDRFBO());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glm::vec4 backgroundColor = camera->GetBackgroundColor();
  glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shadow_cubemap_shader);
  glUniform1i(glGetUniformLocation(shadow_cubemap_shader, "material.diffuse"), 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "view"), 1, GL_FALSE, glm::value_ptr(camera->GetWorldToCameraMatrix()));
  glUniform3fv(glGetUniformLocation(shadow_cubemap_shader, "viewPos"), 1, glm::value_ptr(trans->GetPosition()));
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "far_plane"), camera->GetFarClipPlane());
  for (int i = 0; i < lights.size(); i++) {
    glUniform3fv(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].position").c_str()), 1, glm::value_ptr(movablePointLights[i]));
    glUniform3fv(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].color").c_str()), 1, glm::value_ptr(lights[i].color));
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].attenuation").c_str()), lights[i].attenuation);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].shadowBias").c_str()), lights[i].shadowBias);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].shadowFilterSharpen").c_str()), lights[i].shadowFilterSharpen);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].shadowStrength").c_str()), lights[i].shadowStrength);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].intensity").c_str()), lights[i].intensity);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].castShadow").c_str()), lights[i].castShadow);
    glUniform1f(glGetUniformLocation(shadow_cubemap_shader, ("pointLights[" + std::to_string(i) + "].castTranslucentShadow").c_str()), lights[i].castTranslucentShadow);
    glUniform1i(glGetUniformLocation(shadow_cubemap_shader, ("depthMap[" + std::to_string(i) + "]").c_str()), i + 1);
    glActiveTexture(GL_TEXTURE1 + i);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap[i]);
  }
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "material.shininess"), 128.0f);
  renderScene(shadow_cubemap_shader);
  renderLight();
//  camera->Render();
}

void RenderingEngine::renderLight() {
  glEnable(GL_DEPTH_TEST);
  glUseProgram(normal_shader);
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "view"), 1, GL_FALSE, glm::value_ptr(camera->GetWorldToCameraMatrix()));
  glm::mat4 model = glm::mat4(1.0f);
  for (int i =0; i < lights.size(); i++) {
    model = glm::translate(model, movablePointLights[i]);
    model = glm::scale(model, glm::vec3(0.05f));
    glUniformMatrix4fv(glGetUniformLocation(normal_shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(glGetUniformLocation(normal_shader, "LightColor"), 1, glm::value_ptr(lights[i].color));
    glDrawArrays_profile(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
  }
}

void RenderingEngine::SetSize(int w, int h) {
  width = w;
  height = h;
}

void RenderingEngine::mouseCallback(double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  x_offset = (xpos - lastX) * MouseSensitivity * time->GetDeltaTime();
  y_offset = (lastY - ypos) * MouseSensitivity * time->GetDeltaTime();

  lastX = xpos;
  lastY = ypos;

  trans->Rotate(Transform::Up, -x_offset);
  trans->Rotate(trans->GetRight(), y_offset);
}

void RenderingEngine::keyboardCallback() {
  float speed = 2.5f;
  if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    speed = 7.5f;
  float velocity = speed * time->GetDeltaTime();

  glm::vec3 forward = trans->GetForward();
  glm::vec3 right = trans->GetRight();
  glm::vec3 up = trans->GetUp();
  if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    trans->Translate(forward * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    trans->Translate(-forward * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
    trans->Translate(-right * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
    trans->Translate(right * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
    trans->Translate(up * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
    trans->Translate(-up * velocity);

  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed) {
    camera->SetHdr(!camera->IsHdr());
    hdrKeyPressed = true;
  }
  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_RELEASE) {
    hdrKeyPressed = false;
  }
}