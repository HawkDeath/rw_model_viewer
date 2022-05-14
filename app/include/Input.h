#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <array>
#include <memory>

namespace rw {
class Input : public std::enable_shared_from_this<Input> {
    friend class Window;
public:
    Input() = default;

    int getKeyState(int key) {
        return mKeys.at(key);
    }

private:
    std::array<int, GLFW_KEY_LAST> mKeys;
};
}

#endif // INPUT_H
