
#pragma once

#include "Application.h"

int main(int argc, char* argv[]) {

    Application* amber = new Application();

    amber->create("Amber", 1280, 720);
    amber->update();
    amber->destroy();

    delete amber;
}