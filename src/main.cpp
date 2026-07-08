#include "App.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        App app;
        return app.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}
