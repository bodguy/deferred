#ifndef DEFERRED_POINTLIGHT_H
#define DEFERRED_POINTLIGHT_H

#include <glm/glm.hpp>

class PointLight {
public:
  PointLight(glm::vec3 p, glm::vec3 c);

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

#endif //DEFERRED_POINTLIGHT_H
