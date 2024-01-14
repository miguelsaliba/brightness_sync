#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>
#include <boost/process.hpp>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "main.h"

namespace bp = boost::process;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 87;
    }

    if (!strcmp(argv[1], "up")) {
        return change_brightness(10);
    } else if (!strcmp(argv[1], "down")) {
        return change_brightness(-10);
    }
}

int change_brightness(int change) {
    std::vector<display> displays = {{1, 5, 69}, {2, 20, 100}};

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
        
        bp::child c2("ddcutil setvcp 10 " + std::to_string(value) + " --display " + std::to_string(display.number));
        c2.wait();
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

    c.wait();
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
