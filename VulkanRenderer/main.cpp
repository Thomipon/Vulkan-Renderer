﻿

#include <exception>
#include <iostream>

#include "HelloTriangleApplication.hpp"

int main() {
    try
    {
        HelloTriangleApplication app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
