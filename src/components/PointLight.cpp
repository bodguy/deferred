#include "PointLight.h"
#include "Camera.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

PointLight::PointLight(const glm::vec3& position, const glm::vec3& ambientColor)
  : color(ambientColor), attenuation(0.05f),
    shadowBias(0.01f), shadowFilterSharpen(0.005f), shadowStrength(1.f), nearPlane(0.1f), farPlane(100.f), intensity(0.5f),
    castShadow(true), castTranslucentShadow(true), shadowMapResolution(glm::vec2(512.f, 512.f)), depthCubemap(0), depthCubemapFBO(0), transform(position) {
  normalizedResolution = shadowMapResolution.x / shadowMapResolution.y;
  transform.SetScale(glm::vec3(0.05f));
}

PointLight::~PointLight() {
  glDeleteFramebuffers(1, &depthCubemapFBO);
  glDeleteTextures(1, &depthCubemap);
}

bool PointLight::Init() {
  glGenTextures(1, &depthCubemap);
  glGenFramebuffers(1, &depthCubemapFBO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            shadowMapResolution.x, shadowMapResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
  // only for using depth infomation buffer
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return false;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

Transform* PointLight::GetTransform() {
  return &transform;
}

glm::mat4 PointLight::GetPerspective() const {
  return glm::perspective(glm::radians(90.0f), normalizedResolution, nearPlane, farPlane);
}

glm::mat4 PointLight::GetLookAt(const glm::vec3& forawrdDir, const glm::vec3& upwardDir) const {
  return glm::lookAt(transform.GetPosition(), transform.GetPosition() + forawrdDir, upwardDir);
}

std::vector<glm::mat4> PointLight::GetCubemapShadowMatrix() const {
  std::vector<glm::mat4> mats;
  glm::mat4 perspective = GetPerspective();
  mats.emplace_back(perspective * GetLookAt(Transform::Right, Transform::Down));
  mats.emplace_back(perspective * GetLookAt(Transform::Left, Transform::Down));
  mats.emplace_back(perspective * GetLookAt(Transform::Up, Transform::Backward));
  mats.emplace_back(perspective * GetLookAt(Transform::Down, Transform::Forward));
  mats.emplace_back(perspective * GetLookAt(Transform::Backward, Transform::Down));
  mats.emplace_back(perspective * GetLookAt(Transform::Forward, Transform::Down));
  return mats;
}

void PointLight::RenderLight(unsigned int shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(transform.GetLocalToWorldMatrix()));
  glUniform4fv(glGetUniformLocation(shader, "LightColor"), 1, glm::value_ptr(color));
  glDrawArrays_profile(GL_TRIANGLES, 0, 36);
}

void PointLight::RenderToTexture(unsigned int shader) {
  std::vector<glm::mat4> shadowTransforms = GetCubemapShadowMatrix();
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, shadowMapResolution.x, shadowMapResolution.y);
  glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glUseProgram(shader);
  for (int i = 0; i < shadowTransforms.size(); ++i) {
    glUniformMatrix4fv(glGetUniformLocation(shader, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
  }
  glUniform1f(glGetUniformLocation(shader, "far_plane"), farPlane);
  glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(transform.GetPosition()));
}

void PointLight::BindUniform(unsigned int shader, unsigned int i) const {
  glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].position").c_str()), 1, glm::value_ptr(transform.GetPosition()));
  glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].color").c_str()), 1, glm::value_ptr(color));
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].attenuation").c_str()), attenuation);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].shadowBias").c_str()), shadowBias);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].shadowFilterSharpen").c_str()), shadowFilterSharpen);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].shadowStrength").c_str()), shadowStrength);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].intensity").c_str()), intensity);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].castShadow").c_str()), castShadow);
  glUniform1f(glGetUniformLocation(shader, ("pointLights[" + std::to_string(i) + "].castTranslucentShadow").c_str()), castTranslucentShadow);
  glUniform3fv(glGetUniformLocation(shader, ("lightPos[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(transform.GetPosition()));
  glActiveTexture(GL_TEXTURE2 + i);
  glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
  glUniform1i(glGetUniformLocation(shader, ("depthMap[" + std::to_string(i) + "]").c_str()), 2 + i);
}
