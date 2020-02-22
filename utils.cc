#include <algorithm>
#include <string>

#include "utils.hh"

bool is_mac_address(string str) {
    if (str.size() != 17) {
        return false;
    } else {
        for (int i = 0; i < 17; ++i) {
            if (i % 3 == 2) {
                if (str[i] != ':') {
                    return false;
                }
            } else {
                if (!(('0' <= str[i] && str[i] <= '9') ||
                      ('a' <= str[i] && str[i] <= 'f') ||
                      ('A' <= str[i] && str[i] <= 'F'))) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool is_port_number(string str) {
    if (str.size() > 5) return false;

    for (char c : str) {
        if ('0' <= c && c <= '9') return false;
    }

    if (stoi(str) > 65535) return false;

    return true;
}

unsigned char *to_mac_address(string mac_addr_str) {
    if (mac_addr_str.size() != 17) {
        return nullptr;
    }

    unsigned char *result = new unsigned  char[6];
    fill(result, result + 6, 0);
    for (int i = 0; i < 17; ++i) {
        int off = i / 3;

        if (i % 3 == 2) continue;

        int bit;
        if ('0' <=mac_addr_str[i] && mac_addr_str[i] <= '9') {
            bit = mac_addr_str[i] - '0';
        } else if ('a' <= mac_addr_str[i] && mac_addr_str[i] <= 'f') {
            bit = 10 + mac_addr_str[i] - 'a';
        } else if ('A' <= mac_addr_str[i] && mac_addr_str[i] <= 'F') {
            bit = 10 + mac_addr_str[i] - 'A';
        } else {
            delete[] result;

            return nullptr;
        }

        result[off] |= bit << (1 - i % 3) * 4;
    }

    return result;
}
