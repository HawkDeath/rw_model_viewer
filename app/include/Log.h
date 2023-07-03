#ifndef LOG_H
#define LOG_H

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>
#include <stdexcept>

#define UNUSE(x) (void)x

#define RT_THROW(...) throw std::runtime_error(__VA_ARGS__)

#define VK_CHECK(x, msg) do { auto res = (x); if (res != VK_SUCCESS) { RT_THROW(msg); } } while(0)

#ifdef LOGGER_ENABLED
#define LOG(...) do { spdlog::info(__VA_ARGS__); } while(0)
#define WLOG(...) do { spdlog::warn(__VA_ARGS__); } while(0)
#define ELOG(...) do { spdlog::error(__VA_ARGS__); } while(0)
#else
#define LOG(...) (void)__VA_ARGS__
#define WLOG(...)(void)__VA_ARGS__
#define ELOG(...)(void)__VA_ARGS__
#endif
#endif // LOG_H
