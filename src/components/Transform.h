#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
public:
  Transform();
  ~Transform();

  void Translate(const glm::vec3 pos);
  void Scale(const glm::vec3 scale);
  void Rotate(const glm::vec3 axis, float angle);
  void Rotate(const glm::vec3 eulerAngle);
  glm::vec3 GetPosition() const { return mPosition; }
  glm::vec3 GetScale() const { return mScale; }
  glm::vec3 GetRotation() const { return glm::eulerAngles(mRotation) * 3.14159f / 180.f; }
  glm::mat4 GetLocalToWorldMat() const;

private:
  glm::vec3 mPosition, mScale;
  glm::quat mRotation;
};

#endif // TRANSFORM_H
