#ifndef MATERIAL_H
#define MATERIAL_H

class Material {
public:
  Material() {}
  ~Material() {}

private:
  unsigned int diffuse;
  unsigned int specular;
  float shininess;
};

#endif // MATERIAL_H
