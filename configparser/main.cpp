#include "parser.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage program name + file" << std::endl;
        return 1;
    }
    try
    {
        Parser parser(argv[1]);
        parser.printServerConfigs();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}