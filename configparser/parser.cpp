#include "parser.hpp"

Parser::Parser(const std::string& config_file) : config_file(config_file) 
{
    if (!parseConfigFile()) 
    {
        throw std::runtime_error("Failed to parse config file: " + config_file);
    }
}

Parser::parserConfigFile() 
{
    std::ifstream file(config_file);
    if (!file.is_open()) 
    {
        std::cerr << "Error opening config file: " << config_file << std::endl;
        return false;
    }

    // Parsing logic goes here
    // For example, read lines and populate server_configs vector

    file.close();
    return true;
}

Parser::~Parser() 
{
    // Destructor implementation (if needed)
}
