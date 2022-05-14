#include "DemoApp.h"
#include <Input.h>
#include <Log.h>
#define UNUSE(x) (void)x

namespace app {
DemoApp::DemoApp(int argc, char **argv) {
    UNUSE(argc);
    UNUSE(argv);
}

void DemoApp::run() {
    auto input = mWindow.getInput();
    while(!mWindow.isClose()) {
        glfwPollEvents();

        if (input->getKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            mWindow.close();
        }
    }
}
}
