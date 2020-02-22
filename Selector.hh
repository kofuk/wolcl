#ifndef SELECTOR_HH
#define SELECTOR_HH

#include <string>
#include <vector>

#include <termios.h>

using namespace std;

class Selector {
    vector<string> entries;
    termios orig_termios;
    int total_lines;
    int pos;

public:
    Selector(vector<string> entries);
    ~Selector();

    vector<int> *run();
};

#endif
