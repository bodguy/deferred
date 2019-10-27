#ifndef DEFERRED_FRAMEBUFFER_H
#define DEFERRED_FRAMEBUFFER_H

#include <GL/glew.h>

enum class FramebufferTarget {
  FRAMEBUFFER = GL_FRAMEBUFFER,
  READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER,
  DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER,
};

enum class TextureTarget {
  TEX_2D = GL_TEXTURE_2D,
};

enum class RenderbufferFormat {
  D24_S8 = GL_DEPTH24_STENCIL8,
};

enum class InternalFormat {
  // base internal format
  R = GL_RED,
  RG = GL_RG,
  RGB = GL_RGB,
  BGR = GL_BGR,
  RGBA = GL_RGBA,
  BGRA = GL_BGRA,
  INT_R = GL_RED_INTEGER,
  INT_RG = GL_RG_INTEGER,
  INT_RGB = GL_RGB_INTEGER,
  INT_BGR = GL_BGR_INTEGER,
  INT_RGBA = GL_RGBA_INTEGER,
  INT_BGRA = GL_BGRA_INTEGER,
  DEPTH = GL_DEPTH_COMPONENT,
  DEPTH_STENCIL = GL_DEPTH_STENCIL,
  STENCIL = GL_STENCIL_INDEX,
  // sized internal format
  R8 = GL_R8,
  R8_SNORM = GL_R8_SNORM,
  R16 = GL_R16,
  R16_SNORM = GL_R16_SNORM,
  RG8 = GL_RG8,
  RG8_SNORM = GL_RG8_SNORM,
  RG16 = GL_RG16,
  RG16_SNORM = GL_RG16_SNORM,
  R3G3B2 = GL_R3_G3_B2,
  RGB4 = GL_RGB4,
  RGB5 = GL_RGB5,
  RGB8 = GL_RGB8,
  RGB8_SNORM = GL_RGB8_SNORM,
  RGB10 = GL_RGB10,
  RGB12 = GL_RGB12,
  RGB16_SNORM = GL_RGB16_SNORM,
  RGBA2 = GL_RGBA2,
  RGBA4 = GL_RGBA4,
  RGB5A1 = GL_RGB5_A1,
  RGBA8 = GL_RGBA8,
  RGBA8_SNORM = GL_RGBA8_SNORM,
  RGB10A2 = GL_RGB10_A2,
  RGBA12 = GL_RGBA12,
  RGBA16 = GL_RGBA16,
  SRGB8 = GL_SRGB8,
  SRGBA8 = GL_SRGB8_ALPHA8,
  R16F = GL_R16F,
  RG16F = GL_RG16F,
  RGB16F = GL_RGB16F,
  RGBA16F = GL_RGBA16F,
  R32F = GL_R32F,
  RG32F = GL_RG32F,
  RGB32F = GL_RGB32F,
  RGBA32F = GL_RGBA32F,
  RG11FB10F = GL_R11F_G11F_B10F,
};

enum class PixelDataType {
  UINTB = GL_UNSIGNED_BYTE,
  B = GL_BYTE,
  UINTS = GL_UNSIGNED_SHORT,
  S = GL_SHORT,
  UINT = GL_UNSIGNED_INT,
  INT = GL_INT,
  HF = GL_HALF_FLOAT,
  F = GL_FLOAT,
  UINTB_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
  UINTB_2_3_3_R = GL_UNSIGNED_BYTE_2_3_3_REV,
  UINTS_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
  UINTS_5_6_5_R = GL_UNSIGNED_SHORT_5_6_5_REV,
  UINTS_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
  UINTS_4_4_4_4_R = GL_UNSIGNED_SHORT_4_4_4_4_REV,
  UINTS_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
  UINTS_1_5_5_5_R = GL_UNSIGNED_SHORT_1_5_5_5_REV,
  UINT_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
  UINT_8_8_8_8_R = GL_UNSIGNED_INT_8_8_8_8_REV,
  UINT_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
  UINT_10_10_10_2_R = GL_UNSIGNED_INT_2_10_10_10_REV
};

enum class AttachmentType {
  COLOR = GL_COLOR_ATTACHMENT0,
  DEPTH = GL_DEPTH_ATTACHMENT,
  STENCIL = GL_STENCIL_ATTACHMENT
};

class Texture {
public:
  Texture() : id(0), textureTarget(TextureTarget::TEX_2D), sizedInternalFormat(InternalFormat::RGBA), baseInternalFormat(InternalFormat::RGBA), attachmentType(AttachmentType::COLOR) {
    glGenTextures(1, &id);
  }

  Texture(InternalFormat sizedInternal, InternalFormat baseInternal, PixelDataType dataType, unsigned int w, unsigned int h)
    : id(0), width(w), height(h), textureTarget(TextureTarget::TEX_2D), sizedInternalFormat(sizedInternal), baseInternalFormat(baseInternal), pixelDataType(dataType), attachmentType(AttachmentType::COLOR) {
    glGenTextures(1, &id);
    createEmpty();
  }

  Texture(InternalFormat sizedInternal, InternalFormat baseInternal, PixelDataType dataType, AttachmentType type, unsigned int w, unsigned int h)
          : id(0), width(w), height(h), textureTarget(TextureTarget::TEX_2D), sizedInternalFormat(sizedInternal), baseInternalFormat(baseInternal), pixelDataType(dataType), attachmentType(type) {
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
    glBindTexture(static_cast<GLenum>(textureTarget), id);
    // internalformat: Specifies the number of color components in the texture.
    // format: Specifies the format of the pixel data. (BaseInternalFormat)
    // type: Specifies the data type of the pixel data.
    glTexImage2D(static_cast<GLenum>(textureTarget), 0, static_cast<GLenum>(sizedInternalFormat), w, h, 0, static_cast<GLenum>(baseInternalFormat), static_cast<GLenum>(pixelDataType), ptr);
    glTexParameteri(static_cast<GLenum>(textureTarget), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(static_cast<GLenum>(textureTarget), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(static_cast<GLenum>(textureTarget), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(static_cast<GLenum>(textureTarget), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(static_cast<GLenum>(textureTarget), 0);
  }

  void bind() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(static_cast<GLenum>(textureTarget), id);
  }

  void unbind() {
    glBindTexture(static_cast<GLenum>(textureTarget), 0);
  }

  unsigned int id, width, height;
  TextureTarget textureTarget;
  InternalFormat sizedInternalFormat;
  InternalFormat baseInternalFormat;
  PixelDataType pixelDataType;
  AttachmentType attachmentType;
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
    GLenum att_type;
    if (tex.attachmentType == AttachmentType::COLOR) {
      att_type = static_cast<GLenum>(AttachmentType::COLOR) + last_attachment_count;
      last_attachment_count++;
    } else {
      att_type = static_cast<GLenum>(tex.attachmentType);
      // depth and stencil is not rendering purpose
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
    }
    glFramebufferTexture2D(static_cast<GLenum>(target), att_type, static_cast<GLenum>(tex.textureTarget), tex.id, 0);
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
