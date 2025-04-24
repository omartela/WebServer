#include "parser.hpp"

Parser::Parser(const std::string& config_file) 
{
    if (!parseConfigFile(config_file)) 
    {
        throw std::runtime_error("Failed to parse config file: " + config_file);
    }
}

bool Parser::parseConfigFile(const std::string& config_file)
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
            ServerConfig server_config{}; // alustaa default arvoihin structin siksi kaarisulkeet
            while (getline(file, line) && line.find("}") == std::string::npos)
            {
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
                    if (size_str.find("K") != std::string::npos) 
                    {
                        size_str.erase(size_str.find("K"));
                        server_config.client_max_body_size = std::stoul(size_str) * 1024; // Convert to bytes
                    } 
                    else if (size_str.find("M") != std::string::npos) 
                    {
                        size_str.erase(size_str.find("M"));
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
                    std::string error_page = line.substr(space_pos, end_pos - space_pos);
                    server_config.error_pages[error_code] = error_page;
                }
                else if (line.find("location /") != std::string::npos)
                {
                    Route route{}; // alustaa default arvoihin struktin siksi kaarisulkeet
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
                            size_t pos = line.find("index ") + 6; // Skip "index "
                            size_t end_pos = line.find(";");
                            route.index_file = line.substr(pos, end_pos - pos);
                        }
                        else if (line.find("autoindex ") != std::string::npos)
                        {
                            size_t pos = line.find("autoindex ");
                            size_t end_pos = line.find(";");
                            std::string onoff = line.substr(pos, end_pos - pos);
                            if (onoff.compare("off") == 0)
                                route.autoindex == false;
                            else if (onoff.compare("on") == 0)
                                route.autoindex == true;
                        }
                        else if (line.find("allow_methods ") != std::string::npos)
                        {
                            size_t pos = line.find("allow_methods ") + 14; // Skip "allow_methods "
                            size_t end_pos = line.find(";");
                            std::string methods = line.substr(pos, end_pos - pos);
                            size_t space_pos = 0;
                            while ((space_pos = methods.find(" ")) != std::string::npos) 
                            {
                                route.accepted_methods.push_back(methods.substr(0, space_pos));
                                methods.erase(0, space_pos + 1);
                            }
                            route.accepted_methods.push_back(methods); // Add the last method
                        }
                        else if (line.find("return ") != std::string::npos)
                        {
                            size_t pos = line.find("return ") + 7; // Skip "return "
                            size_t space_pos = line.find(" ", pos);
                            route.redirect.status_code = std::stoi(line.substr(pos, space_pos - pos));
                            route.redirect.target_url = line.substr(space_pos + 1, line.find(";") - space_pos - 1);
                        }
                        else if (line.find("upload_path ") != std::string::npos)
                        {
                            size_t pos = line.find("upload_path ") + 12; // Skip "upload_path "
                            size_t end_pos = line.find(";");
                            route.upload_path = line.substr(pos, end_pos - pos);
                        }
                        else if (line.find("cgi_extension ") != std::string::npos)
                        {
                            size_t pos = line.find("cgi_extension ") + 14; // Skip "cgi_extension "
                            size_t end_pos = line.find(";");
                            route.cgi_extension = line.substr(pos, end_pos - pos);
                        }
                        else if (line.find("upload_path ") != std::string::npos)
                        {
                            size_t pos = line.find("upload_path ") + 12; // Skip "upload_path "
                            size_t end_pos = line.find(";");
                            route.upload_path = line.substr(pos, end_pos - pos);
                        }
                    }
                    server_config.routes.push_back(route);
                }
                // Populate server_config based on the line content
            }
            server_configs.push_back(server_config);
        } 
    }
    file.close();
    return true;
}

std::vector<ServerConfig> Parser::getServerConfigs() 
{
    return server_configs;
}

void Parser::printRoute(const Route& route) const
{
    std::cout << "\033[1;34mPrinting Route struct\033[0m" << std::endl;
    std::cout << "Route Path: " << route.path << std::endl;
    std::cout << "Root: " << route.root << std::endl;
    std::cout << "Accepted Methods: ";
    for (const auto& method : route.accepted_methods) 
    {
        std::cout << method << " ";
    }
    std::cout << std::endl;
    std::cout << "Redirect status code: " << route.redirect.status_code << std::endl;
    std::cout << "Redirect To: " << route.redirect.target_url << std::endl;
    std::cout << "Autoindex: " << (route.autoindex ? "on" : "off") << std::endl;
    std::cout << "Index File: " << route.index_file << std::endl;
    std::cout << "CGI Extension: " << route.cgi_extension << std::endl;
    std::cout << "Upload Path: " << route.upload_path << std::endl;

}

void Parser::printServerConfig(const ServerConfig& server_config) const
{
    std::cout << "\033[1;34mPrinting ServerConfig struct\033[0m" << std::endl;
    std::cout << "Host: " << server_config.host << std::endl;
    std::cout << "Port: " << server_config.port << std::endl;
    std::cout << "Server Names: ";
    for (const auto& name : server_config.server_names) 
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    std::cout << "Client Max Body Size: " << server_config.client_max_body_size << std::endl;
    for (const auto& route : server_config.routes) 
    {
        printRoute(route);
    }
}

void Parser::printServerConfigs() const
{
    for (const auto& server_config : server_configs) 
    {
        printServerConfig(server_config);
    }
}

Parser::~Parser() 
{
    // Destructor implementation (if needed)
}
