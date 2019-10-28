#ifndef DEFERRED_RENDERINGENGINE_H
#define DEFERRED_RENDERINGENGINE_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

struct Character {
    unsigned int TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    signed long Advance;    // Horizontal offset to advance to next glyph
};

struct GLFWwindow;
class RenderingEngine {
public:
    RenderingEngine();
    ~RenderingEngine();

    bool initWindow(const std::string& title, int w, int h);
    void initVertex();
    bool initFramebuffer();
    bool initFont(const std::string& fontpath);
    bool initShader();
    void initTexture();
    int render();
    void renderScene(unsigned int shader);
    void renderFrame();
    void text(std::string text, glm::vec2 pos, glm::vec3 color);

private:
    void mouseCallback(double xpos, double ypos);
    void keyboardCallback();
    void updateDeltaTime();
    void updateCamera();

private:
    GLFWwindow* mWindow;
    glm::vec3 cameraPos, cameraFront, cameraUp, cameraRight;
    float deltaTime, lastFrame, Yaw, Pitch, MouseSensitivity, lastX, lastY;
    bool firstMouse;
    std::map<char, Character> Characters;
    unsigned int font_shader, depth_shader, shadow_shader;
    unsigned int fontVAO, fontVBO;
    unsigned int cubeVAO, cubeVBO;
    int width, height;
    unsigned int wood_texture;
    unsigned int depthMapFBO, depthMap;
};


#endif //DEFERRED_RENDERINGENGINE_H
