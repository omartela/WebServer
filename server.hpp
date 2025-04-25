#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>

#define MAX_CONNECTIONS 1000
#define TIMEOUT 100

void serverLoop();