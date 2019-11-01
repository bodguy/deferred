#include "RenderingEngine.h"

int main() {
  RenderingEngine engine;
  engine.initWindow("LearnOpenGL", 1024, 1024);
  if (!engine.initShader()) {
    std::cout << "shader init failed" << std::endl;
    return -1;
  }
  if (!engine.initFont("../res/arial.ttf")) {
    std::cout << "font init failed" << std::endl;
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