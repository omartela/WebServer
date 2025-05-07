#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <stdexcept>
#include <fcntl.h>

#define MAX_CONNECTIONS 1000
#define TIMEOUT 100

void eventLoop(std::vector<ServerConfig> serverConfigs);