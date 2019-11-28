#include "PointLight.h"

PointLight::PointLight(glm::vec3 p, glm::vec3 c)
  : position(p), color(c), attenuation(0.05f),
    shadowBias(0.01f), shadowFilterSharpen(0.005f), shadowStrength(1.f), nearPlane(0.1f), intensity(0.5f),
    castShadow(true), castTranslucentShadow(true), shadowMapResolution(glm::vec2(512.f, 512.f)) {

}