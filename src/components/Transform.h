#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
public:
  Transform();
  Transform(const glm::vec3& pos);
  ~Transform();

  void Translate(const glm::vec3& pos);
  void Scale(const glm::vec3& s);
  void Rotate(const glm::vec3& axis, float angle);
  glm::vec3 GetPosition() const;
  glm::vec3 GetForward() const;
  glm::vec3 GetUp() const;
  glm::vec3 GetRight() const;
  glm::vec3 GetScale() const;
  glm::quat GetRotation() const;
  glm::mat4 GetLocalToWorldMatrix();
  glm::mat4 GetWorldToLocalMatrix();

  static const glm::vec3 Up, Down, Left, Right, Forward, Backward, One, Zero;

private:
  mutable glm::vec3 position, scale, forward, up, right;
  glm::quat rotation;
  glm::mat4 localToWorldMatrix, worldToLocalMatrix;
};

#endif // TRANSFORM_H
