#include "Graphics.h"
using namespace std;

int main() {
    Graphics graphics;

    while (true) {
        //we usually run our own event loop in OpenGL ES2
        graphics.handleXEvents();
        graphics.draw();
    }
}