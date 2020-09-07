#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

class Material {
  public:
    Material();
    Material(float shine);
    ~Material();

    bool InitDiffuse(const std::string& filename);
    bool InitSpecular(const std::string& filename);
    bool InitNormal(const std::string& filename);

    unsigned int GetDiffuse() const;
    unsigned int GetSpecular() const;
    unsigned int GetNormal() const;
    float GetShininess() const;

    bool GetUseNormal() const;
    void SetUseNormal(bool use);

  private:
    unsigned int diffuse;
    unsigned int specular;
    unsigned int normal;
    bool useNormal;
    float shininess;
};

#endif  // MATERIAL_H
