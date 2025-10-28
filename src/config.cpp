#include "config.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/print_to_stream.hpp>
#include <toml++/impl/table.hpp>
#include <toml++/toml.hpp>

namespace fs = std::filesystem;

fs::path config_filename("brightness.toml");

Config create_config() {
    fs::path config_path;
    if (std::getenv("XDG_CONFIG_HOME")) {
        fs::path xdg_config_home = std::getenv("XDG_CONFIG_HOME");
        config_path = xdg_config_home / config_filename;
    }
    else {
        fs::path home = std::getenv("HOME");
        config_path = home / ".config" / config_filename;
    }

    Config config;
    config.parse_config_file(config_path);
    return config;
}

void Config::parse_config_file(std::string path) {
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    }
    catch (const toml::parse_error& err) {
        std::cerr << "Config parsing error: \n" << err << std::endl;
    }

    std::vector<Display> displays;
    auto display_list = tbl["display"].as_array();
    for (auto& item : *display_list) {
        toml::table disp_table = *item.as_table();
        Display display;
        if (!disp_table["id"].is_value()) {
            std::cerr << "Property 'id' of display table is required." << std::endl;
            std::exit(1);
        }
        display.id = disp_table["id"].as_integer()->get();
        display.min = disp_table["min"].value_or(1);
        display.max = disp_table["max"].value_or(100);

        this->displays.push_back(display);
        disp_table.clear();
    }

    display_list->clear();
}
