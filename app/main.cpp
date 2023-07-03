#include "DemoApp.h"
#include <Log.h>

int main(int argc, char *argv[])
{
    app::DemoApp demo{argc, argv};
    try
    {
        demo.run();
    } catch(std::exception &e)
    {
        ELOG("Throw: {}", e.what());
    }

    return 0;
}
