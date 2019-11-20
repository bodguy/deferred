#ifndef CAMERA_H
#define CAMERA_H

#include "Transform.h"
#include "../util.h"
#include <glm/glm.hpp>

class Camera {
public:
  Camera();
  Camera(const glm::vec3& pos);
  ~Camera();

  bool Init();
  Transform* GetTransform();
  float GetFieldOfView() const;
  glm::vec3 GetBackgroundColor() const;
  float GetNearClipPlane() const;
  float GetFarClipPlane() const;
  float GetAspectRatio() const;
  float GetHdrExposure() const;
  bool IsHdr() const;
  bool IsOrthographic() const;
  Rect<unsigned int> GetPixelRect() const;
  glm::mat4 GetWorldToCameraMatrix();
  glm::mat4 GetCameraToWorldMatrix();
  glm::mat4 GetProjectionMatrix();

  void Render();

  void SetPixelRect(const Rect<unsigned int>& r);
  void SetHdr(bool f);
  void SetOrthographic(bool f);
  void SetFieldOfView(float degree);
  void SetBackgroundColor(const glm::vec3 color);
  void SetNearClipPlane(float near);
  void SetFarClipPlane(float far);
  void SetAspectRatio(int w, int h);
  void SetHdrExposure(float e);

private:
  Transform transform;
  Rect<unsigned int> pixelRect;
  Rect<float> normalizedRect;
  bool hdr;
  bool orthographic;
  float fieldOfView;
  glm::vec3 backgroundColor;
  float nearClipPlane, farClipPlane;
  float aspectRatio;
  float exposure;
  unsigned int targetTexture; // @TODO
  bool useOcclusionCulling; // @TODO
  glm::mat4 worldToCameraMatrix, cameraToWorldMatrix, projectionMatrix;
  unsigned int quadVAO, quadVBO, hdrFBO, hdrColorTexture, hdrRboDepth, hdrShader;
};


#endif // CAMERA_H
