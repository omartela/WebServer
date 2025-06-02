#pragma once

#include "eventLoop.hpp"
#include "Client.hpp"

void checkTimeouts(int timerFd, std::map<int, Client>& clients, int& children, int loop);
void checkChildrenStatus(int timerFd, std::map<int, Client>& clients, int loop, int& children);
void handleSIGPIPE(int signum);