#include "main.h"
#include <boost/process.hpp>
#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ddcutil_types.h>
#include <iostream>
#include <string>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/print_to_stream.hpp>
#include <toml++/impl/table.hpp>
#include <toml++/toml.hpp>
#include <unistd.h>
#include <vector>

// TODO: add a slider to show the percentages with the min and max.
namespace bp = boost::process;

bool is_number(char *c) { return isdigit(*c) || *c == '-' || *c == '+'; }

int main(int argc, char *argv[]) {
    if (argc == 1) {
        help();
        return 87;
    }
    std::vector<display> displays = get_displays();
    char *cmd = argv[1];

    if (!strcmp(cmd, "up")) {
        change_brightness(10, displays);
    } else if (!strcmp(cmd, "down")) {
        change_brightness(-10, displays);
    } else if (!strcmp(cmd, "set") || (*cmd == 's' && cmd[1] == 0)) {
        if (!is_number(argv[2])) {
            std::cerr << "Please enter a valid number" << std::endl;
            return 1;
        }
        set_brightness(std::stoi(argv[2]), displays);
    } else if (!strcmp(cmd, "change") || (*cmd == 'c' && cmd[1] == 0)) {
        if (!is_number(argv[2])) {
            std::cerr << "Please enter a valid number" << std::endl;
            return 1;
        }
        change_brightness(std::stoi(argv[2]), displays);
    } else if (!strcmp(cmd, "get") || (*cmd == 'g' && cmd[1] == 0)) {
        print_brightness(displays);
    } else if (!strcmp(cmd, "help")) {
        help();
    } else {
        std::cerr << "Command not found. Please run 'brightness_sync help'"
                  << std::endl;
        return 0;
    }

    return 0;
}

int change_brightness(int change, std::vector<display> &displays) {
    display primary_display = displays[0];
    int primary_brightness;
    try {
        primary_brightness = get_brightness(primary_display.id);
    } catch (int error_code) {
        std::cerr << "ddcutil failed with error code: " << error_code
                  << ". Please make sure ddcutil is on your path and is "
                     "working correctly first."
                  << std::endl;
        return error_code;
    }
    double brightness_percentage =
        (primary_brightness - primary_display.min) /
        (double)(primary_display.max - primary_display.min);
    brightness_percentage *= 100;
    brightness_percentage += (double)change;

    if (brightness_percentage < 0) {
        brightness_percentage = 0;
    } else if (brightness_percentage > 100) {
        brightness_percentage = 100;
    }

    set_brightness(brightness_percentage, displays);
    return 0;
}

void set_brightness(double percentage, std::vector<display> &displays) {
    if (percentage < 0) {
        percentage = 0;
    } else if (percentage > 100) {
        percentage = 100;
    }

    std::cout << "Current brightness: " << (int)(percentage) << '%'
              << std::endl;

    for (auto display : displays) {
        int value =
            display.min + (percentage / 100 * (display.max - display.min));
        std::cout << "Display " << display.id << ": " << value << '%'
                  << std::endl;

        bp::child(
            bp::search_path("ddcutil"),
            "setvcp", "10", std::to_string(value),
            "--display", std::to_string(display.id)
        ).join();
    }
}

// Gets the brightness of the given ddcutil display number.
// Throws if ddcutil fails to return a brightness percentage.
int get_brightness(int display_number) {
    bp::ipstream is;
    // NOTE: add support for custom hex for brightness in config file.
    bp::child c(
        bp::search_path("ddcutil"),
        "getvcp", "10",
        "--brief",
        "--display" + std::to_string(display_number),
        bp::std_out > is, bp::std_err > bp::null
    );

    std::string l;
    std::string line;
    while (is && std::getline(is, l) && !l.empty() && l.length() > 1) {
        line = l;
    }

    c.join();
    int result = c.exit_code();
    if (result != 0) {
        throw result;
    }

    std::istringstream ss(line);
    // ignores the first 3 words
    std::string ignore;
    ss >> ignore;
    ss >> ignore;
    ss >> ignore;

    // gets the brightness value from the output.
    int number;
    ss >> number;
    return number;
}

void print_brightness(std::vector<display> displays) {
    std::cout << "Current brightness" << std::endl;

    for (const auto &display : displays) {
        std::cout << "Display " << display.id << ": "
                  << get_brightness(display.id) << '%' << std::endl;
    }
}

std::vector<display> get_displays() {
    std::string config_location = "brightness.toml";
    if (std::getenv("XDG_CONFIG_HOME") != NULL) {
        config_location = '/' + config_location;
        config_location = std::getenv("XDG_CONFIG_HOME") + config_location;
    } else {
        config_location = "/.config/" + config_location;
        config_location = std::getenv("$HOME") + config_location;
    }

    toml::table tbl;
    try {
        tbl = toml::parse_file(config_location);
    } catch (const toml::parse_error &err) {
        std::cerr << "Config parsing error: \n" << err << std::endl;
    }
    auto display_list = tbl["display"].as_array();
    std::vector<display> displays;
    for (auto &item : *display_list) {
        toml::table display_table = *item.as_table();
        display display = {
            static_cast<int>(display_table["id"].as_integer()->get()),
            static_cast<int>(display_table["min"].as_integer()->get()),
            static_cast<int>(display_table["max"].as_integer()->get()),
        };
        display_table.clear();
        displays.emplace_back(display);
        continue;
    }
    display_list->clear();
    return displays;
}

void help() {
    std::cout
        << "Usage: brightness_sync [command] [value]\n"
           "Commands:\n"
           "   help                    prints this help page\n"
           "   up                      increases the brightness by 10%\n"
           "   down                    decreases the brightness by 10%\n"
           "   s, set VALUE            sets the brightness to the value\n"
           "   c, change VALUE         changes the brightness by the value\n"
           "   g, get VALUE            gets the brightness of all the "
           "monitors\n"
        << std::endl;
}
