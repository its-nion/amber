#include "Application.h"

int main(int argc, char* argv[])
{
    try {
        Application app("amber", 1280, 720);
        app.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Application encountered an error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}