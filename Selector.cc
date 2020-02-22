#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "Selector.hh"

Selector::Selector(vector<string> entries) : entries(entries) {
    if (!isatty(0)) {
        throw runtime_error("stdin is not a tty");
    }

    if (tcgetattr(0, &orig_termios) == -1) {
        throw runtime_error(strerror(errno));
    }

    termios new_termios = orig_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(0, TCIFLUSH, &new_termios);
}

Selector::~Selector() {
    write(1, "\r", 1);
    for (int i = 0; i < total_lines - pos; ++i) {
        write(1, "\e[B", 3);
    }

    tcsetattr(0, TCIFLUSH, &orig_termios);
}

vector<int> *Selector::run() {
    vector<int> *result = new vector<int>();

    for (string const &ent : entries) {
        cout << "( ) " << ent << endl;
        ++total_lines;
    }

    cout << endl;
    cout << "Arrow : move        Space: select or unselect" << endl;
    cout << "Return: execute     q    : exit" << endl;

    total_lines += 3;
    write(1, "\e[C", 3);
    for (int i = 0; i < entries.size() + 3; ++i) {
        write(1, "\e[A", 3);
    }

    pos = 0;

    char c;
    for (;;) {
        read(0, &c, 1);

        if (c == 'q') {
            delete result;

            return nullptr;
        } else if (c == ' ') {
            auto res = find(begin(*result), end(*result), pos);
            if (res == end(*result)) {
                result->push_back(pos);

                write(1, "*\e[D", 4);
                if (pos < entries.size() - 1) {
                    write(1, "\e[B", 3);
                    ++pos;
                }
            } else {
                result->erase(res);

                write(1, " \e[D", 4);
                if (pos < entries.size() - 1) {
                    write(1, "\e[B", 3);
                    ++pos;
                }
            }
        } else if (c == '\e') {
            read(0, &c, 1);
            if (c != '[') {
                continue;
            }

            read(0, &c, 1);
            if (c == 'A') {
                if (pos > 0) {
                    --pos;
                    write(1, "\e[A", 3);
                }
            } else if (c == 'B') {
                if (pos < entries.size() - 1) {
                    ++pos;
                    write(1, "\e[B", 3);
                }
            }
        } else if (c == 'N' - '@' || c == 'j') {
            if (pos < entries.size() - 1) {
                ++pos;
                write(1, "\e[B", 3);
            }
        } else if (c == 'P' - '@' || c == 'k') {
            if (pos > 0) {
                --pos;
                write(1, "\e[A", 3);
            }
        } else if (c == '\n' || c == 'M' - '@') {
            break;
        }
    }

    sort(begin(*result), end(*result));

    return result;
}
