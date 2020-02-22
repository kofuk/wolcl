#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <vector>

#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include <readline/readline.h>

#include "Selector.hh"
#include "WOLHandler.hh"
#include "utils.hh"

using namespace std;

static void configure_global(string config_path, string addr, string port) {
    ofstream config(config_path);
    if (!config) {
        cerr << "cannot create config file" << endl;

        exit(1);
    }

    Json::Value val;
    val["addr"] = addr;
    val["port"] = port;
    val["clients"] = Json::Value(Json::arrayValue);

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    unique_ptr<Json::StreamWriter> w(builder.newStreamWriter());
    w->write(val, &config);
    config << endl;
}

static void generate_default_config(string config_path) {
    configure_global(config_path, "192.168.1.255", "7");
}

static void register_mode(string config_path) {
    char *line;

    cout << "Register a new client" << endl;

    string name;
    string mac;
    string addr;
    string port;

    for (;;) {
        line = readline("Name: ");
        if (line != nullptr) {
            name = line;

            break;
        }
    }

    for (;;) {
        line = readline("MAC address (NN:NN:NN:NN:NN:NN): ");
        if (line != nullptr) {

            if (is_mac_address(line)) {
                mac = line;

                break;
            }
        }
    }

    line = readline("Set client-specific host and port settings?[y/N] ");
    if (line != nullptr && (!strcmp(line, "y") || !strcmp(line, "Y"))) {
        for (;;) {
            line = readline("Host: ");
            if (line != nullptr) {
                addr = line;

                break;
            }
        }

        for (;;) {
            line = readline("Port[0-65535]: ");
            if (is_port_number(line)) {
                port = line;

                break;
            }
        }
    }

    ifstream in_file(config_path);

    if (!in_file) {
        cerr << "Cannot open config file" << endl;

        exit(1);
    }

    Json::CharReaderBuilder ibuilder;
    ibuilder["collectComments"] = false;
    Json::Value config;
    string errs;
    bool ok = Json::parseFromStream(ibuilder, in_file, &config, &errs);

    Json::Value new_value;
    new_value["name"] = name;
    new_value["mac_addr"] = mac;
    new_value["addr"] = addr;
    new_value["port"] = port;

    config["clients"].append(new_value);

    ofstream out_file(config_path);
    if (!out_file) {
        cout << "cannot write to config file" << endl;
    }

    Json::StreamWriterBuilder obuilder;
    obuilder["indentation"] = "  ";
    unique_ptr<Json::StreamWriter> w(obuilder.newStreamWriter());
    w->write(config, &out_file);
    out_file << endl;
}

int main(int argc, char **argv) {
    passwd *p = getpwuid(getuid());
    if (p == nullptr) {
        perror("wolcl: getpwuid()");

        return 1;
    }

    string config_path = string(p->pw_dir) + "/.wolcl.json";

    if (argc > 1 && !strcmp(argv[1], "register")) {
        register_mode(config_path);

        return 0;
    }

    struct stat st;
    if (stat(config_path.c_str(), &st) == -1) {
        if (errno == ENOENT) {
            generate_default_config(config_path);
        } else {
            perror("wolcl: cannot stat config file");

            return 1;
        }
    }

    ifstream config_file(config_path);

    if (!config_file) {
        cerr << "Cannot open config file" << endl;

        return 1;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::Value config;
    string errs;
    bool ok = Json::parseFromStream(builder, config_file, &config, &errs);

    if (!ok) {
        cout << errs << endl;

        return 1;
    }

    string addr = config["addr"].asString();
    string port = config["port"].asString();
    Json::Value clients = config["clients"];
    if (addr.empty() || port.empty()) {
        cerr << "Global `addr' or `port' is not specified" << endl;

        return 1;
    }

    if (clients.empty()) {
        cout << "No clients registered:" << endl;
        cout << "Run `wolcl register' to register" << endl;

        return 0;
    }

    vector<string> entries;
    for (Json::Value const &client : clients) {
        string name = client["name"].asString();
        entries.push_back(name);
    }

    Selector *select = new Selector(entries);
    vector<int> *selected = select->run();

    delete select;

    if (selected == nullptr) {
        return 0;
    }

    cout << endl;

    try {
        WOLHandler *handler = new WOLHandler(addr, port);

        for (int pos : *selected) {
            string laddr = clients[pos]["addr"].asString();
            string lport = clients[pos]["port"].asString();

            unsigned char *mac =
                to_mac_address(clients[pos]["mac_addr"].asString());
            if (mac == nullptr) {
                cout << "cannot parse mac address: "
                     << clients[pos]["mac_addr"].asString() << endl;

                continue;
            }

            cout << "Sending Wake-on-LAN to "
                 << clients[pos]["mac_addr"].asString() << "..." << endl;

            if (!laddr.empty() || !port.empty()) {
                if (laddr.empty()) {
                    laddr = addr;
                }
                if (lport.empty()) {
                    lport = port;
                }
                WOLHandler *handler = new WOLHandler(laddr, lport);
                handler->send(mac);
                delete handler;
            } else {
                handler->send(mac);
            }
        }

        delete handler;
    } catch (runtime_error const &e) {
        cerr << e.what() << endl;
        delete selected;

        return 1;
    }

    delete selected;

    return 0;
}
