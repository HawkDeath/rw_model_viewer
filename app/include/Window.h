#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace rw {
class Input;
class Window {
public:
    Window(const std::string &title, const std::int32_t &width, const int32_t &height);
    Window(const Window&) = delete;
    Window(Window &&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();

    // Vulkan stuffs
    void createSurface(VkInstance instance, VkSurfaceKHR *surface);

    // Window stuffs
    bool isClose() const { return glfwWindowShouldClose(mWindow); }
    bool wasResized() const { return mWasResized; }
    void resetSizeState() { mWasResized = false; }
    void close() const { glfwSetWindowShouldClose(mWindow, GLFW_TRUE);}
    glm::ivec2 size() const {
        return glm::ivec2(mWidth, mHeight);
    }

    std::shared_ptr<Input> getInput() const {
        return mInput;
    }
private:
    static void framebuffer_size_callback(GLFWwindow *win, int width, int height);
    static void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods);
private:
    GLFWwindow *mWindow;
    int32_t mWidth;
    int32_t mHeight;
    bool mWasResized;

    std::shared_ptr<Input> mInput;
};
}

#endif // WINDOW_H
