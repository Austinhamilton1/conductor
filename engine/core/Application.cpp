#include "Application.h"

void conductor::Application::run() {
    running = true;

    initialize();

    while(running) update();

    shutdown();
}

void conductor::Application::stop() {
    running = false;
}

bool conductor::Application::is_running() const {
    return running;
}