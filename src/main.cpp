#include "application.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        calculator::Application application;
        application.run(argc, argv);
    }
    catch (const std::exception& ec)
    {
        calculator::Logger::getInstance().error(ec.what());
        return EXIT_FAILURE;
    }
    catch (const std::string& str)
    {
        std::cout << str << '\n';
    }
    catch (...)
    {
        calculator::Logger::getInstance().error("Unknown error\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
