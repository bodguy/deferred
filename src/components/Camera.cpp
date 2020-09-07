#include "Camera.h"

#include <GL/glew.h>

#include "../RenderingEngine.h"

Camera::Camera()
    : transform(),
      pixelRect(),
      normalizedRect(),
      hdr(false),
      orthographic(false),
      fieldOfView(glm::radians(45.f)),
      backgroundColor(0.f),
      nearClipPlane(0.1f),
      farClipPlane(100.f),
      aspectRatio(0.f),
      exposure(1.f),
      targetTexture(0),
      useOcclusionCulling(false),
      worldToCameraMatrix(),
      cameraToWorldMatrix(),
      projectionMatrix(),
      quadVAO(0),
      quadVBO(0),
      hdrFBO(0),
      hdrColorTexture(0),
      hdrRboDepth(0),
      hdrShader(0) {}

Camera::Camera(const glm::vec3& pos)
    : transform(pos),
      pixelRect(),
      normalizedRect(),
      hdr(false),
      orthographic(false),
      fieldOfView(glm::radians(45.f)),
      backgroundColor(0.f),
      nearClipPlane(0.1f),
      farClipPlane(100.f),
      aspectRatio(0.f),
      exposure(1.f),
      targetTexture(0),
      useOcclusionCulling(false),
      worldToCameraMatrix(),
      cameraToWorldMatrix(),
      projectionMatrix(),
      quadVAO(0),
      quadVBO(0),
      hdrFBO(0),
      hdrColorTexture(0),
      hdrRboDepth(0),
      hdrShader(0) {}

Camera::~Camera() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(1, &hdrColorTexture);
    glDeleteRenderbuffers(1, &hdrRboDepth);
    glDeleteProgram(hdrShader);
}

bool Camera::Init() {
    SetPixelRect(Rect<unsigned int>(0, 0, RenderingEngine::GetInstance()->GetWidth(), RenderingEngine::GetInstance()->GetHeight()));
    SetAspectRatio(pixelRect.w, pixelRect.h);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(utils::quadVertices), &utils::quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glGenTextures(1, &hdrColorTexture);
    glBindTexture(GL_TEXTURE_2D, hdrColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, pixelRect.w, pixelRect.h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glGenRenderbuffers(1, &hdrRboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, hdrRboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, pixelRect.w, pixelRect.h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdrRboDepth);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrColorTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    hdrShader = loadShaderFromFile("../shaders/hdr/hdr_vs.shader", "../shaders/hdr/hdr_fs.shader");
    return hdrShader != 0;
}

Transform* Camera::GetTransform() { return &transform; }

float Camera::GetFieldOfView() const { return fieldOfView; }

glm::vec4 Camera::GetBackgroundColor() const { return backgroundColor; }

float Camera::GetNearClipPlane() const { return nearClipPlane; }

float Camera::GetFarClipPlane() const { return farClipPlane; }

float Camera::GetAspectRatio() const { return aspectRatio; }

float Camera::GetHdrExposure() const { return exposure; }

bool Camera::IsHdr() const { return hdr; }

bool Camera::IsOrthographic() const { return orthographic; }

Rect<unsigned int> Camera::GetPixelRect() const { return pixelRect; }

glm::mat4 Camera::GetWorldToCameraMatrix() {
    glm::vec3 pos = transform.GetPosition();
    worldToCameraMatrix = glm::lookAt(pos, pos + transform.GetForward(), transform.GetUp());
    return worldToCameraMatrix;
}

glm::mat4 Camera::GetCameraToWorldMatrix() {
    cameraToWorldMatrix = glm::inverse(GetWorldToCameraMatrix());
    return cameraToWorldMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() {
    if (orthographic) {
        projectionMatrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, nearClipPlane, farClipPlane);
    } else {
        projectionMatrix = glm::perspective(fieldOfView, (float)pixelRect.w / (float)pixelRect.h, nearClipPlane, farClipPlane);
    }

    return projectionMatrix;
}

void Camera::Render() {
    glViewport(pixelRect.x, pixelRect.y, pixelRect.w, pixelRect.h);
    glBindFramebuffer(GL_FRAMEBUFFER, targetTexture);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(hdrShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorTexture);
    glUniform1i(glGetUniformLocation(hdrShader, "hdr"), hdr);
    glUniform1f(glGetUniformLocation(hdrShader, "exposure"), exposure);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Camera::SetPixelRect(const Rect<unsigned int>& r) {
    pixelRect.x = r.x;
    pixelRect.y = r.y;
    pixelRect.w = r.w;
    pixelRect.h = r.h;
    normalizedRect.x = (float)pixelRect.x / (float)pixelRect.w;
    normalizedRect.y = (float)pixelRect.y / (float)pixelRect.h;
    normalizedRect.w = (float)pixelRect.w / (float)pixelRect.w;
    normalizedRect.h = (float)pixelRect.h / (float)pixelRect.h;
}

void Camera::SetHdr(bool f) { hdr = f; }

void Camera::SetOrthographic(bool f) { orthographic = f; }

void Camera::SetFieldOfView(float degree) { fieldOfView = glm::radians(degree); }

void Camera::SetBackgroundColor(const glm::vec4 color) { backgroundColor = color; }

void Camera::SetNearClipPlane(float near) { nearClipPlane = near; }

void Camera::SetFarClipPlane(float far) { farClipPlane = far; }

void Camera::SetAspectRatio(int w, int h) { aspectRatio = (float)w / (float)h; }

void Camera::SetHdrExposure(float e) { exposure = e; }

unsigned int Camera::GetHDRFBO() const { return hdrFBO; }
