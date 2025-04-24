#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <iostream>
#include <fstream>

// Erilaisia redirect status koodeja ja käyttötarkoituksia
/*
Koodi | Nimi | Käyttötarkoitus
301 | Moved Permanently | Pysyvä uudelleenohjaus
302 | Found (aiemmin "Moved Temporarily") | Tilapäinen uudelleenohjaus
307 | Temporary Redirect | Kuten 302, mutta metodit säilyvät
308 | Permanent Redirect | Kuten 301, mutta metodit säilyvät
*/

struct Redirect 
{
    int status_code;              // Status koodi redirectionille
    std::string target_url;       // Esim. url uuteen osoitteeseen esim /new
};

struct Route
{
    std::string path;
    std::string root;
    std::vector<std::string> accepted_methods;
    Redirect redirect;
    bool autoindex;
    std::string index_file;
    std::string cgi_extension;
    std::string upload_path;
};

struct ServerConfig 
{
    std::string host;
    int port;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    std::vector<Route> routes;
};

class Parser
{

    private:
        std::vector<ServerConfig> server_configs;
    public:
        Parser(const std::string& config_file);
        Parser(const Parser& src) = delete; // Disable copy constructor
        Parser& operator=(const Parser& src) = delete; // Disable copy assignment operator¨
        ~Parser();
        bool parseConfigFile(const std::string& config_file);
        std::vector<ServerConfig> getServerConfigs();
        void printServerConfigs() const;
        void printRoutes() const;
        void printRoute(const Route& route) const;
        void printServerConfig(const ServerConfig& server_config) const;
};
