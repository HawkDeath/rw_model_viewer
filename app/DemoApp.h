#ifndef DEMOAPP_H
#define DEMOAPP_H

#include <Window.h>
#include <render/Device.h>

namespace app
{
class DemoApp
{
public:
    DemoApp(int argc, char **argv);
    ~DemoApp() = default;

    void run();

private:
    rw::Window mWindow {"rw_model_viewer", 1280, 720};
    rw::Device mDevice {mWindow};
};
}

#endif // DEMOAPP_H
