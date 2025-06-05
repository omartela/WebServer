#pragma once
#include "HTTPRequest.hpp"
#include <atomic>
#include <csignal>
#include <signal.h>

std::string join_paths(std::filesystem::path path1, std::filesystem::path path2);
bool validateHeader(HTTPRequest req);
void handleSignals(int signum);

extern std::atomic<int> signum;