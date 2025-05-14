#pragma once
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/timerfd.h>
#include <cstdlib>
#include <stdexcept>
#include "Parser.hpp"

#define MAX_CONNECTIONS 1000
#define TIMEOUT 10

void eventLoop(std::vector<ServerConfig> serverConfigs);