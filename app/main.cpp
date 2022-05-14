#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cstdint>

int main(int argc, char *argv[]) {
    spdlog::info("Hello world");
    std::uint32_t vkVersion;
    vkEnumerateInstanceVersion(&vkVersion);

    glfwInit();
    GLFWwindow *win = glfwCreateWindow(1280, 720, "", nullptr, nullptr);

    spdlog::info("GLFW {}.{}, {}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
    spdlog::info("Vulkan Version: {}.{}.{}", VK_VERSION_MAJOR(vkVersion), VK_VERSION_MINOR(vkVersion), VK_VERSION_PATCH(vkVersion));

    while(!glfwWindowShouldClose(win)) {
        glfwPollEvents();
    }
    if (win) {
        glfwDestroyWindow(win);
    }
    glfwTerminate();
    return 0;
}
