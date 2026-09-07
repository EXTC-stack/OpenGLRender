#include "Core/Application.h"
#include "Core/Scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

static void FramebufferSizeCallback(GLFWwindow* win, int w, int h) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(win));
    app->m_width = w; app->m_height = h;
    glViewport(0, 0, w, h);
}

bool Application::Init(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_width = width; m_height = height;
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "GLFW 窗口创建失败\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD 初始化失败\n";
        return false;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";
    glEnable(GL_DEPTH_TEST);
    return true;
}

void Application::Run(Scene& scene, const std::function<void(float)>& onUpdate) {
    float lastTime = 0.0f;
    while (!glfwWindowShouldClose(m_window)) {
        float now = (float)glfwGetTime();
        float dt = now - lastTime;
        lastTime = now;

        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(m_window, true);

        scene.Update(dt);
        if (onUpdate) onUpdate(dt);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.Render(m_width, m_height);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Application::Shutdown() {
    glfwTerminate();
}