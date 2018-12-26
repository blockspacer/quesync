#pragma once
#include <SDKDDKVer.h>
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WINXP // Define min sockets version of Windows XP

#include <iostream>
#include <WinSock2.h>
#include <ws2def.h>
#include <ws2tcpip.h>

#include "socket-error.hpp"

class SocketManager
{
public:
    static void initWinsock();

    static SOCKET createSocket(const char *ipAddress, const char *port, bool isTCP, bool nonBlocking);
};
