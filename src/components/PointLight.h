#ifndef DEFERRED_POINTLIGHT_H
#define DEFERRED_POINTLIGHT_H

#include "Transform.h"
#include <glm/glm.hpp>
#include <vector>

class Camera;
class PointLight {
public:
  PointLight(const glm::vec3& position, const glm::vec3& ambientColor);
  ~PointLight();

  bool Init();
  Transform* GetTransform();
  glm::mat4 GetPerspective() const;
  std::vector<glm::mat4> GetCubemapShadowMatrix() const;
  void Render(unsigned int vao, unsigned int shader, Camera* cam);
  void RenderToTexture(unsigned int shader);
  void BindUniform(unsigned int shader, unsigned int i) const;

private:
  glm::mat4 GetLookAt(const glm::vec3& forawrdDir, const glm::vec3& upwardDir) const;

private:
  glm::vec3 color;
  float attenuation;
  float shadowBias;
  float shadowFilterSharpen;
  float shadowStrength;
  float nearPlane;
  float farPlane;
  float intensity;
  bool castShadow;
  bool castTranslucentShadow;
  glm::vec2 shadowMapResolution; // immutable
  unsigned int depthCubemap;
  unsigned int depthCubemapFBO;
  Transform transform;
  float normalizedResolution; // immutable
};

#endif //DEFERRED_POINTLIGHT_H
