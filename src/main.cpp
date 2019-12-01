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
  engine.initVertex();
  return engine.render();
}