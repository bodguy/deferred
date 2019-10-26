#ifndef DEFERRED_FRAMEBUFFER_H
#define DEFERRED_FRAMEBUFFER_H

#include <GL/glew.h>

enum class FramebufferTarget {
  FRAMEBUFFER = GL_FRAMEBUFFER,
  READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER,
  DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER,
};

enum class TextureTarget {
  TEXTURE2D = GL_TEXTURE_2D,
};

enum class RenderbufferFormat {
  D24_S8 = GL_DEPTH24_STENCIL8,
};

class Texture {
public:
  Texture(TextureTarget target) : id(0), textarget(target) {
    glGenTextures(1, &id);
  }

  Texture(TextureTarget target, unsigned int w, unsigned int h) : id(0), width(w), height(h), textarget(target) {
    glGenTextures(1, &id);
    createEmpty();
  }

  ~Texture() {
    glDeleteTextures(1, &id);
  }

  void createEmpty() {
    create(width, height, nullptr);
  }

  void create(unsigned int w, unsigned int h, void* ptr) {
    glBindTexture(static_cast<GLenum>(textarget), id);
    glTexImage2D(static_cast<GLenum>(textarget), 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, ptr);
    glTexParameteri(static_cast<GLenum>(textarget), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(static_cast<GLenum>(textarget), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(static_cast<GLenum>(textarget), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(static_cast<GLenum>(textarget), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(static_cast<GLenum>(textarget), 0);
  }

  void bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(static_cast<GLenum>(textarget), id);
  }

  void unbind() {
    glBindTexture(static_cast<GLenum>(textarget), 0);
  }

  unsigned int id, width, height;
  TextureTarget textarget;
};

class Renderbuffer {
public:
  Renderbuffer(RenderbufferFormat fmt, unsigned int width, unsigned int height) : id(0), format(fmt), w(width), h(height) {
    glGenRenderbuffers(1, &id);
    storage();
  }

  ~Renderbuffer() {
    glDeleteRenderbuffers(1, &id);
  }

  void storage() {
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(format), w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  void bind() {
    glBindRenderbuffer(GL_RENDERBUFFER, id);
  }

  void unbind() {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  unsigned int id;
  RenderbufferFormat format;
  unsigned int w, h;
};

class Framebuffer {
public:
  Framebuffer(FramebufferTarget t) : id(0), target(t), last_attachment_count(0) {
    glGenFramebuffers(1, &id);
  }

  ~Framebuffer() {
    glDeleteFramebuffers(1, &id);
  }

  void attach(const Texture& tex) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferTexture2D(static_cast<GLenum>(target), GL_COLOR_ATTACHMENT0 + last_attachment_count, static_cast<GLenum>(tex.textarget), tex.id, 0);
    last_attachment_count++;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void attach(const Renderbuffer& rb) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb.id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
  }

  void unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  bool check() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
  }

  unsigned int id;
  FramebufferTarget target;
  unsigned int last_attachment_count;
};

#endif //DEFERRED_FRAMEBUFFER_H
