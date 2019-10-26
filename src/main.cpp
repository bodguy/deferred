#include <iostream>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include "util.h"
#include "framebuffer.h"

void render_text(unsigned int shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 right = glm::vec3();

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Euler Angles
float Yaw = -90.0f;
float Pitch = 0.0f;
float MouseSensitivity = 0.1f;

bool wire_frame_toggle = false;
bool firstMouse = true;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
  GLuint TextureID;   // ID handle of the glyph texture
  glm::ivec2 Size;    // Size of glyph
  glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
  signed long Advance;    // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
GLuint fontVAO, fontVBO;

unsigned int blur_w = 1280;
unsigned int blur_h = 720;

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK) return -1;

  // Set OpenGL options
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  unsigned int normal_shader = loadShaderFromFile("../shaders/normal/normal_vs.shader", "../shaders/normal/normal_fs.shader");
  unsigned int font_shader = loadShaderFromFile("../shaders/font/font_vs.shader", "../shaders/font/font_fs.shader");
  unsigned int hblur_shader = loadShaderFromFile("../shaders/guassian_blur/hblur_vs.shader", "../shaders/guassian_blur/blur_fs.shader");
  unsigned int vblur_shader = loadShaderFromFile("../shaders/guassian_blur/vblur_vs.shader", "../shaders/guassian_blur/blur_fs.shader");
  unsigned int bright_shader = loadShaderFromFile("../shaders/bloom/bloom_vs.shader", "../shaders/bloom/brighter_fs.shader");
  unsigned int combine_shader = loadShaderFromFile("../shaders/bloom/bloom_vs.shader", "../shaders/bloom/combine_fs.shader");
  glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
  glUseProgram(font_shader);
  glUniformMatrix4fv(glGetUniformLocation(font_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  // FreeType
  FT_Library ft;
  // All functions return a value different than 0 whenever an error occurred
  if (FT_Init_FreeType(&ft))
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

  // Load font as face
  FT_Face face;
  if (FT_New_Face(ft, "../res/arial.ttf", 0, &face))
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

  // Set size to load glyphs as
  FT_Set_Pixel_Sizes(face, 0, 48);

  // Disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Load first 128 characters of ASCII set
  for (GLubyte c = 0; c < 128; c++) {
    // Load character glyph
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
      continue;
    }
    // Generate texture
    GLuint font_texture;
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use
    Character character = {
            font_texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
    };
    Characters.insert(std::pair<GLchar, Character>(c, character));
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  // Destroy FreeType once we're finished
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  // Configure VAO/VBO for texture quads
  glGenVertexArrays(1, &fontVAO);
  glGenBuffers(1, &fontVBO);
  glBindVertexArray(fontVAO);
  glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // make vao, vbo
  unsigned int cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  // plane VAO
  unsigned int planeVAO, planeVBO;
  glGenVertexArrays(1, &planeVAO);
  glGenBuffers(1, &planeVBO);
  glBindVertexArray(planeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  // screen quad VAO
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  unsigned int cube_texture = loadTexture("../res/wooden.jpg");
  unsigned int floor_texture = loadTexture("../res/metal.png");

  Texture textureList[4] = {
      Texture(TextureTarget::TEXTURE2D, SCR_WIDTH, SCR_HEIGHT),
      Texture(TextureTarget::TEXTURE2D, SCR_WIDTH, SCR_HEIGHT),
      Texture(TextureTarget::TEXTURE2D, SCR_WIDTH, SCR_HEIGHT),
      Texture(TextureTarget::TEXTURE2D, SCR_WIDTH, SCR_HEIGHT)
  };

  Renderbuffer renderbufferList[4] = {
      Renderbuffer(RenderbufferFormat::D24_S8, SCR_WIDTH, SCR_HEIGHT),
      Renderbuffer(RenderbufferFormat::D24_S8, SCR_WIDTH, SCR_HEIGHT),
      Renderbuffer(RenderbufferFormat::D24_S8, SCR_WIDTH, SCR_HEIGHT),
      Renderbuffer(RenderbufferFormat::D24_S8, SCR_WIDTH, SCR_HEIGHT)
  };

  Framebuffer framebufferList[4] = {
      Framebuffer(FramebufferTarget::FRAMEBUFFER),
      Framebuffer(FramebufferTarget::FRAMEBUFFER),
      Framebuffer(FramebufferTarget::FRAMEBUFFER),
      Framebuffer(FramebufferTarget::FRAMEBUFFER)
  };

  for (int i = 0; i < 4; ++i) {
      framebufferList[i].attach(textureList[i]);
      framebufferList[i].attach(renderbufferList[i]);
      if (!framebufferList[i].check()) return -1;
  }

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Calculate the new Front vector
    glm::vec3 f;
    f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    f.y = sin(glm::radians(Pitch));
    f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));
    up = glm::normalize(glm::cross(right, front));

    processInput(window);

    // first pass
    {
      framebufferList[0].bind();
      glEnable(GL_DEPTH_TEST);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(normal_shader);
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = glm::lookAt(pos, pos + front, up);
      glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
      glUniformMatrix4fv(glGetUniformLocation(normal_shader, "view"), 1, GL_FALSE, &view[0][0]);
      glUniformMatrix4fv(glGetUniformLocation(normal_shader, "projection"), 1, GL_FALSE, &projection[0][0]);

      // first cube
      glBindVertexArray(cubeVAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, cube_texture);
      model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
      glUniformMatrix4fv(glGetUniformLocation(normal_shader, "model"), 1, GL_FALSE, &model[0][0]);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      // another cube
      model = glm::translate(model, glm::vec3(3.0f, 0.0f, 2.0f));
      glUniformMatrix4fv(glGetUniformLocation(normal_shader, "model"), 1, GL_FALSE, &model[0][0]);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      // floor
      glBindVertexArray(planeVAO);
      glBindTexture(GL_TEXTURE_2D, floor_texture);
      glUniformMatrix4fv(glGetUniformLocation(normal_shader, "model"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glBindVertexArray(0);
    }

    // second pass
    {
      framebufferList[1].bind();
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindVertexArray(quadVAO);
      textureList[0].bind();

      glUseProgram(hblur_shader);
      glUniform1f(glGetUniformLocation(hblur_shader, "targetWidth"), blur_w / 2);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 3. third pass
    {
      framebufferList[2].bind();
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindVertexArray(quadVAO);
      textureList[1].bind();
      glUseProgram(vblur_shader);
      glUniform1f(glGetUniformLocation(vblur_shader, "targetHeight"), blur_h / 2);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 4. fourth pass
    {
      framebufferList[3].bind();
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindVertexArray(quadVAO);
      textureList[2].bind();
      glUseProgram(hblur_shader);
      glUniform1f(glGetUniformLocation(hblur_shader, "targetWidth"), blur_w / 4);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // 3. last pass (default framebuffer)
    {
      glDisable(GL_DEPTH_TEST);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glBindVertexArray(quadVAO);
      textureList[3].bind();
      glUseProgram(vblur_shader);
      glUniform1f(glGetUniformLocation(vblur_shader, "targetHeight"), blur_h / 4);
      glDrawArrays(GL_TRIANGLES, 0, 6);


      render_text(font_shader, "blur_x: " + std::to_string(blur_w) + ", blur_y: " + std::to_string(blur_h), 5.0f, 5.0f, 0.5f, glm::vec3(1.f, 0.f, 0.f));
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteVertexArrays(1, &planeVAO);
  glDeleteVertexArrays(1, &quadVAO);
  glDeleteBuffers(1, &cubeVBO);
  glDeleteBuffers(1, &planeVBO);
  glDeleteBuffers(1, &quadVBO);

  glDeleteProgram(normal_shader);
  glDeleteProgram(font_shader);
  glDeleteProgram(hblur_shader);
  glDeleteProgram(vblur_shader);
  glDeleteProgram(bright_shader);
  glDeleteProgram(combine_shader);

  for (auto& i : Characters) {
    glDeleteTextures(1, &i.second.TextureID);
  }
  glDeleteTextures(1, &cube_texture);
  glDeleteTextures(1, &floor_texture);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  float velocity = 2.5f * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    pos += front * velocity;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    pos -= front * velocity;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    pos -= right * velocity;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    pos += right * velocity;
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
    wire_frame_toggle = !wire_frame_toggle;
    if (wire_frame_toggle) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }
  int step = 50;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    if (blur_w > 0) {
      blur_w-=step;
    }
    if (blur_h > 0) {
      blur_h-=step;
    }
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    if (blur_w < 2480) {
      blur_w+=step;
    }
    if (blur_h < 1440) {
      blur_h+=step;
    }
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
  if (state == GLFW_PRESS) {
    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw   += xoffset;
    Pitch += yoffset;

    if (Pitch > 89.0f)
      Pitch = 89.0f;
    if (Pitch < -89.0f)
      Pitch = -89.0f;
  }
}

void render_text(unsigned int shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
  glUseProgram(shader);
  glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(fontVAO);

  // Iterate through all characters
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++)
  {
    Character ch = Characters[*c];

    GLfloat xpos = x + ch.Bearing.x * scale;
    GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

    GLfloat w = ch.Size.x * scale;
    GLfloat h = ch.Size.y * scale;
    // Update VBO for each character
    GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
    };
    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);
    // Update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}