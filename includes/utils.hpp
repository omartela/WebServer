#pragma once
#include "HTTPRequest.hpp"
#include <atomic>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);
bool validateHeader(HTTPRequest req);
void handleSIGPIPE(int signum);

extern std::atomic<int> eventFD;
extern std::atomic<int> signum;
