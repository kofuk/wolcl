#ifndef WOLHANDLER_HH
#define WOLHANDLER_HH

#include <string>

#include <sys/socket.h>

using namespace std;

class WOLHandler {
    int sock;
    sockaddr addr;
    socklen_t addr_len;

public:
    WOLHandler(string broadcast_addr, string port);
    ~WOLHandler();

    void send(unsigned char *mac_addr);
};

#endif
