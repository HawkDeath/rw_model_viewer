cmake_minimum_required(VERSION 3.10)

set(APP_RENDER_SRC
    src/render/Device.cpp
    src/render/PhysicalDevice.cpp)

set(APP_RENDER_HPP
    include/render/Device.h
    include/render/PhysicalDevice.h)

set(APP_SRC
    src/Window.cpp
    DemoApp.cpp)

set(APP_HPP
    include/Log.h
    include/Input.h
    include/Window.h
    DemoApp.h)


set(APP_SOURCES ${APP_SRC} ${APP_HPP} ${APP_RENDER_SRC} ${APP_RENDER_HPP})

add_executable(rw_model_viewer main.cpp ${APP_SOURCES})
target_link_libraries(rw_model_viewer PRIVATE glfw glm spdlog vulkan imgui)
target_include_directories(rw_model_viewer PRIVATE include)
target_compile_definitions(rw_model_viewer PRIVATE -DLOGGER_ENABLED)
