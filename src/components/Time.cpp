#include "Time.h"

#include <GLFW/glfw3.h>

Time::Time() : deltaTime(0.0f), lastFrame(0.0f) {}

void Time::Update() {
    auto currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

float Time::GetDeltaTime() const { return deltaTime; }

float Time::ElapsedTime() const { return lastFrame; }
