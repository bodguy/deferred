#include "RenderingEngine.h"

int main() {
  RenderingEngine engine;
  if (!engine.initWindow("Three point light shadow mapping example", 1280, 720)) {
    std::cout << "window init failed" << std::endl;
    return -1;
  }
  if (!engine.initShader()) {
    std::cout << "shader init failed" << std::endl;
    return -1;
  }
  if (!engine.initTexture()) {
    std::cout << "texture init failed" << std::endl;
    return -1;
  }
  engine.initVertex();
  if (!engine.initFramebuffer()) {
    std::cout << "framebuffer init failed" << std::endl;
    return -1;
  }
  return engine.render();
}