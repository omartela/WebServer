#pragma once

#include "eventLoop.hpp"
#include "Client.hpp"

void checkTimeouts(int timerFd, std::map<int, Client>& clients, int loop);