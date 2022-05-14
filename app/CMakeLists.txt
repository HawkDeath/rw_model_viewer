cmake_minimum_required(VERSION 3.10)


set(APP_SRC
)

set(APP_HPP
)


set(APP_SOURCES ${APP_SRC} ${APP_HPP})

add_executable(rw_model_viewer main.cpp ${APP_SOURCES})
target_link_libraries(rw_model_viewer PRIVATE glfw glm spdlog vulkan imgui)
target_include_directories(rw_model_viewer PRIVATE include)
