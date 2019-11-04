#include "Transform.h"

Transform::Transform() {

}

Transform::~Transform() {

}

void Transform::Translate(const glm::vec3 pos) {
  mPosition += pos;
}

void Transform::Scale(const glm::vec3 scale) {
  mScale += scale;
}

void Transform::Rotate(const glm::vec3 axis, float angle) {
  mRotation = glm::angleAxis(angle, axis);
}

void Transform::Rotate(const glm::vec3 eulerAngle) {

}

glm::mat4 Transform::GetLocalToWorldMat() const {
  return glm::mat4(1.f);
}