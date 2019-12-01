#include "RenderingEngine.h"
#include "util.h"
#include "components/Material.h"
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
          normal_shader(0), depth_cubemap_shader(0), shadow_cubemap_shader(0),
          cubeVAO(0), cubeVBO(0), planeVAO(0), planeVBO(0),
          width(0), height(0),
          gpuTimeProfileQuery(0), timeElapsed(0), hdrKeyPressed(false), x_offset(0.f), y_offset(0.f),
          fontRenderer(new FontRenderer()), camera(new Camera(glm::vec3(0.0f, 2.3f, 8.0f))), time (new Time()),
          cameraTrans(nullptr), cube1(nullptr), cube2(nullptr), lights() {
  instance = this;
  lights.clear();
}

RenderingEngine::~RenderingEngine() {
  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteBuffers(1, &planeVBO);

  glDeleteProgram(normal_shader);
  glDeleteProgram(depth_cubemap_shader);
  glDeleteProgram(shadow_cubemap_shader);
  glDeleteQueries(1, &gpuTimeProfileQuery);

  SAFE_DEALLOC(fontRenderer);
  SAFE_DEALLOC(camera);
  SAFE_DEALLOC(time);
  SAFE_DEALLOC(cube1);
  SAFE_DEALLOC(cube2);
  for (auto pl : lights) {
    SAFE_DEALLOC(pl);
  }
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

  mWindow = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
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
    std::cout << "fontRenderer Init failed" << std::endl;
    return false;
  }

  if (!camera->Init()) {
    std::cout << "camera Init failed" << std::endl;
    return false;
  }
  cameraTrans = camera->GetTransform();

  cube1 = new Material(128.f);
  if (!cube1->InitDiffuse("../res/wood.png")) return false;
  cube2 = new Material(128.f);
  if (!cube2->InitDiffuse("../res/brickwall.jpg")) return false;
  if (!cube2->InitNormal("../res/brickwall_normal.jpg")) return false;

  PointLight* light1 = new PointLight(glm::vec3( 3.17f,  2.34f,  -4.184f), glm::vec3(1.f, 1.f, 1.f));
  if (!light1->Init()) {
    std::cout << "light1 Init failed" << std::endl;
    return false;
  }
  PointLight* light2 = new PointLight(glm::vec3( 2.3f, 2.f, -4.0f),  glm::vec3(1.f, 1.f, 1.f));
  if (!light2->Init()) {
    std::cout << "light2 Init failed" << std::endl;
    return false;
  }
  PointLight* light3 = new PointLight(glm::vec3(2.3f, 2.f, -8.0f), glm::vec3(1.f, 1.f, 1.f));
  if (!light3->Init()) {
    std::cout << "light3 Init failed" << std::endl;
    return false;
  }
  lights.emplace_back(light1);
  lights.emplace_back(light2);
  lights.emplace_back(light3);

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

bool RenderingEngine::initShader() {
  normal_shader = loadShaderFromFile("../shaders/normal/normal_vs.shader", "../shaders/normal/normal_fs.shader");
  if (!normal_shader) return false;
  depth_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/depth_vs.shader", "../shaders/point_shadow/depth_gs.shader", "../shaders/point_shadow/depth_fs.shader");
  if (!depth_cubemap_shader) return false;
  shadow_cubemap_shader = loadShaderFromFile("../shaders/point_shadow/shadow_vs.shader", "../shaders/point_shadow/shadow_fs.shader");
  if (!shadow_cubemap_shader) return false;
  return true;
}

int RenderingEngine::render() {
  while (!glfwWindowShouldClose(mWindow)) {
    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(mWindow, true);

    time->Update();
    keyboardCallback();

    glBeginQuery(GL_TIME_ELAPSED, gpuTimeProfileQuery);
    renderFrame();
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(gpuTimeProfileQuery, GL_QUERY_RESULT, &timeElapsed);

    renderFont();
    resetProfile();
    glfwSwapBuffers(mWindow);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void RenderingEngine::renderFont() {
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
  glm::vec3 f = cameraTrans->GetForward();
  fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 1), "camera front: [%.2f, %.2f, %.2f]", f.x, f.y, f.z);
  glm::vec3 p = cameraTrans->GetPosition();
  fontRenderer->Printf(glm::vec2(5.f, 5 + 22 * 0), "camera pos: [%.2f, %.2f, %.2f]", p.x, p.y, p.z);
}

void RenderingEngine::renderScene(unsigned int shader) {
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cube1->GetDiffuse());
  glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
  glUniform1f(glGetUniformLocation(shader, "material.shininess"), cube1->GetShininess());

  // floor
  glm::mat4 model = glm::mat4(1.0f);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glBindVertexArray(planeVAO);
  glDrawArrays_profile(GL_TRIANGLES, 0, 6);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // first cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 1.5f, -8.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // another cube
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(2.0f, 0.0f, -6.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // another cube2
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -4.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.25));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cube2->GetDiffuse());
  glUniform1f(glGetUniformLocation(shader, "material.shininess"), cube2->GetShininess());

  // cube1
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.92f, 0.f, -3.f));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // cube2
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-4.0f, 0.0f, -2.0));
  model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
  // cube3
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0));
  model = glm::scale(model, glm::vec3(0.5f));
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
}

void RenderingEngine::renderFrame() {
  // 1. drawing geometry to depth cube map
  for (int i = 0; i < lights.size(); i++) {
    lights[i]->GetTransform()->SetPosition(glm::vec3(cos(time->ElapsedTime() * (0.5f * (i + 1))) * 5.f, 3, sin(time->ElapsedTime() * (0.5f * (i + 1))) * 5.f));
    lights[i]->RenderToTexture(depth_cubemap_shader);
    renderScene(depth_cubemap_shader);
  }

  // 2. drawing to the hdr floating point framebuffer
  glViewport(0, 0, width, height);
  glBindFramebuffer(GL_FRAMEBUFFER, camera->GetHDRFBO());
  glm::vec4 backgroundColor = camera->GetBackgroundColor();
  glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(shadow_cubemap_shader);
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(shadow_cubemap_shader, "view"), 1, GL_FALSE, glm::value_ptr(camera->GetWorldToCameraMatrix()));
  glUniform3fv(glGetUniformLocation(shadow_cubemap_shader, "viewPos"), 1, glm::value_ptr(cameraTrans->GetPosition()));
  glUniform1f(glGetUniformLocation(shadow_cubemap_shader, "far_plane"), camera->GetFarClipPlane());
  for (int i = 0; i < lights.size(); i++) {
    lights[i]->BindUniform(shadow_cubemap_shader, i);
  }
  renderScene(shadow_cubemap_shader);
  glEnable(GL_DEPTH_TEST);
  glUseProgram(normal_shader);
  glBindVertexArray(cubeVAO);
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "projection"), 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));
  glUniformMatrix4fv(glGetUniformLocation(normal_shader, "view"), 1, GL_FALSE, glm::value_ptr(camera->GetWorldToCameraMatrix()));
  for (auto& light : lights) {
    light->RenderLight(normal_shader);
  }
  camera->Render();
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

  cameraTrans->Rotate(Transform::Up, -x_offset);
  cameraTrans->Rotate(cameraTrans->GetRight(), y_offset);
}

void RenderingEngine::keyboardCallback() {
  float speed = 2.5f;
  if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    speed = 7.5f;
  float velocity = speed * time->GetDeltaTime();

  glm::vec3 forward = cameraTrans->GetForward();
  glm::vec3 right = cameraTrans->GetRight();
  glm::vec3 up = cameraTrans->GetUp();
  if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    cameraTrans->Translate(forward * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    cameraTrans->Translate(-forward * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
    cameraTrans->Translate(-right * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
    cameraTrans->Translate(right * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS)
    cameraTrans->Translate(up * velocity);
  if (glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS)
    cameraTrans->Translate(-up * velocity);

  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed) {
    camera->SetHdr(!camera->IsHdr());
    hdrKeyPressed = true;
  }
  if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_RELEASE) {
    hdrKeyPressed = false;
  }
}