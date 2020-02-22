#include <cstring>
#include <stdexcept>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "WOLHandler.hh"

using namespace std;

WOLHandler::WOLHandler(string broadcast_addr, string port) : sock(-1) {
    addrinfo hints;
    addrinfo *results;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    int err_code;
    if ((err_code = getaddrinfo(broadcast_addr.c_str(), port.c_str(), &hints,
                                &results)) != 0) {
        throw runtime_error(gai_strerror(err_code));
    }

    for (addrinfo *addr = results; addr != nullptr; addr = addr->ai_next) {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock == -1) {
            continue;
        } else {
            int flag = 1;
            setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(int));

            this->addr = *addr->ai_addr;
            this->addr_len = addr->ai_addrlen;

            break;
        }
    }

    if (sock == -1) {
        throw runtime_error("cannot create socket");
    }

    freeaddrinfo(results);
}

WOLHandler::~WOLHandler() {
    if (sock != -1) {
        close(sock);
    }
}

void WOLHandler::send(unsigned char *mac_addr) {
    unsigned char packet[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    int off = 6;
    for (; off < 102; ++off) {
        packet[off] = mac_addr[off % 6];
    }

    if (sendto(sock, packet, 102, 0, &addr, addr_len) < 102) {
        int error = errno;
        throw runtime_error(string("wol-util: sendto() :") + strerror(error));
    }
}
