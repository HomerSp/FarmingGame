#include <ui/sdlwindow.h>

int main(int argc, char** argv)
{
    SdlWindow window;
    if (!window.init(1280, 720)) {
        return -1;
    }
    window.run();
    return 0;
}
