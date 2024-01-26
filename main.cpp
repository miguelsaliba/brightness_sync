#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>
#include <boost/process.hpp>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <toml++/toml.hpp>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/table.hpp>
#include <toml++/impl/array.hpp>
#include <toml++/impl/print_to_stream.hpp>
#include <unistd.h>
#include <vector>
#include "main.h"

namespace bp = boost::process;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 87;
    }
    std::vector<display> displays = get_displays();

    if (!strcmp(argv[1], "up")) {
        change_brightness(10, displays);
    }
    else if (!strcmp(argv[1], "down")) {
        change_brightness(-10, displays);
    }
    else if (isdigit(*argv[1]) || *argv[1] == '-') {
        change_brightness(atoi(argv[1]), displays);
    }
    
    return 0;
}

int change_brightness(int change, std::vector<display> displays) {

    // Brightness of the primary display (maybe should use xrandr to detect primary display instead?).
    display primary_display = displays[0];
    int primary_brightness = get_brightness(primary_display.number);
    double brightness_percentage = (primary_brightness - primary_display.min) / (double) (primary_display.max - primary_display.min);
    brightness_percentage += (double) change / 100;

    if (brightness_percentage < 0) {
        brightness_percentage = 0;
    } else if (brightness_percentage > 1) {
        brightness_percentage = 1;
    }

    std::cout << "Current brightness: " << (int) (brightness_percentage * 100) << '%' << std::endl;

    for (auto display : displays) {
        int value = display.min + (brightness_percentage * (display.max - display.min));
        std::cout << "Display " << display.number << ": " << value << '%' << std::endl;

        bp::child c2("ddcutil setvcp 10 " + std::to_string(value) + " --display " + std::to_string(display.number));
        c2.join();
    }
    return 0;
}

int get_brightness(int display_number) {
    bp::ipstream is;
    // NOTE: add support for custom hex for brightness in config file.
    bp::child c("ddcutil getvcp 10 --brief --display " + std::to_string(display_number), bp::std_out > is);

    std::string l;
    std::string line;
    while (is && std::getline(is, l) && !l.empty() && l.length() > 1) {
        line = l;
    }

    c.join();
    int result = c.exit_code();
    if (result != 0) {
        std::cerr << line << std::endl;
        return result;
    }

    std::size_t space_index = std::distance(line.begin(), boost::algorithm::find_nth(line, " ", 2).begin());
    if (space_index <= 0) {
        std::cout << "something happened idk" << std::endl;
        std::cout << "space_index: " << space_index << std::endl;
    }
    int value = atoi(&line.at(space_index));
    return value;
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
    }
    catch (const toml::parse_error& err) {
        std::cerr << "Config parsing error: \n" << err << std::endl;
    }
    auto display_list = tbl["display"].as_array();
    std::vector<display> displays;
    for (auto& item : *display_list) {
        toml::table display_table = *item.as_table();
        display display = {
            static_cast<int>(display_table["number"].as_integer()->get()),
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
