#include <glm/gtc/matrix_transform.hpp>
#include "Transform.h"

const glm::vec3 Transform::Up = glm::vec3(0.f, 1.f, 0.f);
const glm::vec3 Transform::Down = glm::vec3(0.f, -1.f, 0.f);
const glm::vec3 Transform::Left = glm::vec3(-1.f, 0.f, 0.f);
const glm::vec3 Transform::Right = glm::vec3(1.f, 0.f, 0.f);
const glm::vec3 Transform::Forward = glm::vec3(0.f, 0.f, -1.f);
const glm::vec3 Transform::Backward = glm::vec3(0.f, 0.f, 1.f);
const glm::vec3 Transform::One = glm::vec3(1.f, 1.f, 1.f);
const glm::vec3 Transform::Zero = glm::vec3(0.f, 0.f, 0.f);

Transform::Transform() : position(0.f), scale(1.f), rotation(), localToWorldMatrix(), worldToLocalMatrix() {
  forward = rotation * Transform::Forward;
  up = rotation * Transform::Up;
  right = rotation * Transform::Right;
}

Transform::Transform(const glm::vec3& pos) : position(pos), scale(1.f), rotation(), localToWorldMatrix(), worldToLocalMatrix() {
  forward = rotation * Transform::Forward;
  up = rotation * Transform::Up;
  right = rotation * Transform::Right;
}

Transform::~Transform() {

}

void Transform::Translate(const glm::vec3& pos) {
  position += pos;
}

void Transform::Scale(const glm::vec3& s) {
  scale += s;
}

void Transform::Rotate(const glm::vec3& axis, float angle) {
  rotation = glm::normalize(glm::angleAxis(angle, glm::normalize(axis)) * rotation);
}

glm::vec3 Transform::GetPosition() const {
  return position;
}

glm::vec3 Transform::GetForward() const {
  forward = GetRotation() * Transform::Forward;
  return glm::normalize(forward);
}

glm::vec3 Transform::GetUp() const {
  up = GetRotation() * Transform::Up;
  return glm::normalize(up);
}

glm::vec3 Transform::GetRight() const {
  right = glm::cross(GetForward(), GetUp());
  return glm::normalize(right);
}

glm::vec3 Transform::GetScale() const {
  return scale;
}

glm::quat Transform::GetRotation() const {
  return rotation;
}

glm::mat4 Transform::GetLocalToWorldMatrix() {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model = model * glm::toMat4(rotation);
  model = glm::scale(model, scale);
  localToWorldMatrix = model;
  return localToWorldMatrix;
}

glm::mat4 Transform::GetWorldToLocalMatrix() {
  worldToLocalMatrix = glm::inverse(GetLocalToWorldMatrix());
  return worldToLocalMatrix;
}

void Transform::SetPosition(const glm::vec3 &pos) {
  position = pos;
}

void Transform::SetScale(const glm::vec3 &s) {
  scale = s;
}
