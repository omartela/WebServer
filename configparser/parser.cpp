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

    std::string line;
    while (getline(file, line)) 
    {
        // Check if the line contains "server {" with a space in between
        if (line.find("server {") != std::string::npos) 
        {
            while (getline(file, line) && line.find("}") == std::string::npos)
            {
                ServerConfig server_config;
                if (line.find("listen ") != std::string::npos)
                {
                    // Extract host and port from the line
                    size_t pos = line.find("listen ") + 7; // Skip "listen "
                    size_t end_pos = line.find(";", pos);
                    std::string host_port = line.substr(pos, end_pos - pos);
                    size_t colon_pos = host_port.find(":");
                    if (colon_pos != std::string::npos) 
                    {
                        server_config.host = host_port.substr(0, colon_pos);
                        server_config.port = std::stoi(host_port.substr(colon_pos + 1));
                    } 
                    else 
                    {
                        // If no port is specified, assume default HTTP port 80
                        // not sure if not specied host can you only specify port
                        server_config.host = host_port;
                        server_config.port = 80; // Default port
                    }
                }
                else if (line.find("server_name ") != std::string::npos)
                {
                    // Extract server names from the line
                    size_t pos = line.find("server_name ") + 12; // Skip "server_name "
                    size_t end_pos = line.find(";", pos);
                    std::string server_names = line.substr(pos, end_pos - pos);
                    size_t space_pos = 0;   
                    while ((space_pos = server_names.find(" ")) != std::string::npos) 
                    {
                        server_config.server_names.push_back(server_names.substr(0, space_pos));
                        server_names.erase(0, space_pos + 1);
                    }
                    server_config.server_names.push_back(server_names); // Add the last name
                }
                else if (line.find("client_max_body_size ") != std::string::npos)
                {
                    // Extract client_max_body_size from the line
                    size_t pos = line.find("client_max_body_size ") + 21; // Skip "client_max_body_size "
                    size_t end_pos = line.find(";", pos);
                    std::string size_str = line.substr(pos, end_pos - pos);
                    if (size_str.find("k") != std::string::npos) 
                    {
                        size_str.erase(size_str.find("k"));
                        server_config.client_max_body_size = std::stoul(size_str) * 1024; // Convert to bytes
                    } 
                    else if (size_str.find("m") != std::string::npos) 
                    {
                        size_str.erase(size_str.find("m"));
                        server_config.client_max_body_size = std::stoul(size_str) * 1024 * 1024; // Convert to bytes
                    } 
                    else 
                    {
                        server_config.client_max_body_size = std::stoul(size_str); // Assume bytes
                    }
                }
                else if (line.find("error_page ") != std::string::npos)
                {
                    // Extract error pages from the line
                    size_t pos = line.find("error_page ") + 11; // Skip "error_page "
                    size_t end_pos = line.find(";", pos); // need to check npos if ; not found
                    size_t space_pos = line.find(" ", pos);
                    int error_code = std::stoi(line.substr(pos, space_pos - pos));
                    std::string error_page = line.substr(space_pos + 1, end_pos - space_pos - 1);
                    server_config.error_pages[error_code] = error_page;
                }
                else if (line.find("location /") != std::string::npos)
                {
                    Route route;
                    size_t pos = line.find("location /") + 10; // Skip "location /"
                    size_t end_pos = line.find("{");
                    route.path = line.substr(pos, end_pos - pos);
                    while (getline(file, line) && line.find("}") == std::string::npos)
                    {
                        if (line.find("root ") != std::string::npos)
                        {
                            size_t pos = line.find("root ") + 5; // Skip "root "
                            size_t end_pos = line.find(";");
                            route.root = line.substr(pos, end_pos - pos);
                        }
                        else if (line.find("index ") != std::string::npos)
                        {

                        }
                        else if (line.find("autoindex ") != std::string::npos)
                        {
                            
                        }
                        else if (line.find("allow_methods ") != std::string::npos)
                        {

                        }
                    }
                }

                 // Populate server_config based on the line content
                 server_configs.push_back(serve_config);
            }
        } 
        else if (line.find("route {") != std::string::npos) 
        {
            Route route;
            // Populate route based on the line content
            server_configs.back().routes.push_back(route);
        }
    }


    file.close();
    return true;
}

Parser::~Parser() 
{
    // Destructor implementation (if needed)
}
