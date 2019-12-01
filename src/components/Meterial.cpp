#include "Material.h"
#include "../util.h"

Material::Material()
  : diffuse(0), specular(0), normal(0), shininess(128.f) {

}

Material::Material(float shin)
  : diffuse(0), specular(0), normal(0), shininess(shin) {

}

Material::~Material() {
  if (!diffuse) {
    glDeleteTextures(1, &diffuse);
  }
  if (!specular) {
    glDeleteTextures(1, &specular);
  }
  if (!normal) {
    glDeleteTextures(1, &normal);
  }
}

bool Material::InitDiffuse(const std::string &filename) {
  diffuse = loadTexture(filename.c_str(), false);
  return diffuse != 0;
}

bool Material::InitSpecular(const std::string &filename) {
  specular = loadTexture(filename.c_str(), false);
  return specular != 0;
}

bool Material::InitNormal(const std::string &filename) {
  normal = loadTexture(filename.c_str(), false);
  return normal != 0;
}

unsigned int Material::GetDiffuse() const {
  return diffuse;
}

unsigned int Material::GetSpecular() const {
  return specular;
}

unsigned int Material::GetNormal() const {
  return normal;
}

float Material::GetShininess() const {
  return shininess;
}
