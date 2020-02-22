#ifndef UTILS_HH
#define UTILS_HH

#include <string>

using namespace std;

bool is_mac_address(string str);
bool is_port_number(string str);

unsigned char *to_mac_address(string mac_address_string);

#endif
