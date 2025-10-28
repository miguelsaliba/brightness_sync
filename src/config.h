#pragma once

#include <string>
#include <vector>

struct Display {
    int id;
    int min;
    int max;
};

struct Config {
    std::vector<Display> displays;

    void parse_config_file(std::string path);
};

Config create_config();
