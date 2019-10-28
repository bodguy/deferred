#include "RenderingEngine.h"

int main() {
  RenderingEngine engine;
  engine.initWindow("LearnOpenGL", 1024, 768);
  engine.initShader();
  engine.initFont("../res/arial.ttf");
  engine.initTexture();
  engine.initVertex();
  engine.initFramebuffer();
  return engine.render();
}