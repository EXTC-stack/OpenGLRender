#pragma once
#include <functional>

struct GLFWwindow;
class Scene;

class Application {
public:
    bool Init(int width, int height, const char* title);
    // onUpdate：每帧回调（放你的游戏逻辑，比如旋转立方体）
    void Run(Scene& scene, const std::function<void(float dt)>& onUpdate = nullptr);
    void Shutdown();

    GLFWwindow* GetWindow() const { return m_window; }
    float GetAspect() const { return (float)m_width / (float)m_height; }

private:
    friend void FramebufferSizeCallback(GLFWwindow*, int, int); 
    GLFWwindow* m_window = nullptr;
    int m_width = 0, m_height = 0;
};