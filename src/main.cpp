#include <boost/process/io.hpp>
#include <boost/process/pipe.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>
#include <boost/process.hpp>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <toml++/toml.hpp>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/table.hpp>
#include <toml++/impl/array.hpp>
#include <toml++/impl/print_to_stream.hpp>
#include <ddcutil_types.h>
#include <ddcutil_c_api.h>
#include "main.h"

// TODO: add a slider to show the percentages with the min and max.
namespace bp = boost::process;

DDCA_Vcp_Feature_Code feature_code = 0x10;

bool is_number(char *c) {
    return isdigit(*c) || *c == '-' || *c == '+';
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        help();
        return 87;
    }
    std::vector<Display> displays = get_displays();
    char* cmd = argv[1];
    char* value = argv[2];

    if (!strcmp(cmd, "up")) {
        change_brightness(10, displays);
    }
    else if (!strcmp(cmd, "down")) {
        change_brightness(-10, displays);
    }
    else if ((!strcmp(cmd, "set") || (*cmd == 's' && cmd[1] == 0)) && argc == 3) {
        if (!is_number(value)) {
            std::cerr << "Please enter a valid number" << std::endl;
            return 1;
        }
        set_brightness(std::stoi(value), displays);
    }
    else if ((!strcmp(cmd, "change") || (*cmd == 'c' && cmd[1] == 0)) && argc == 3) {
        if (!is_number(value)) {
            std::cerr << "Please enter a valid number" << std::endl;
            return 1;
        }
        change_brightness(std::stoi(value), displays);
    }
    else if (!strcmp(cmd, "get") || (*cmd == 'g' && cmd[1] == 0)) {
        print_brightness(displays);
    }
    else if (!strcmp(cmd, "help")) {
        help();
    }
    else {
        std::cerr << "Command not found. Please run 'brightness_sync help'" << std::endl;
        return 0;
    }

    return 0;
}

int change_brightness(int change, std::vector<Display> &displays) {
    // Brightness of the primary display (maybe should use xrandr to detect primary display instead?).
    Display primary_display = displays[0];
    int primary_brightness;
    try {
        primary_brightness = get_brightness(primary_display);
    } catch (int error_code) {
        std::cerr << "ddcutil failed with error code: " << error_code << ". Please make sure ddcutil is on your path and is working correctly first." << std::endl;
        return error_code;
    }
    double brightness_percentage = (primary_brightness - primary_display.min) / (double) (primary_display.max - primary_display.min);
    brightness_percentage *= 100;
    brightness_percentage += (double) change;

    if (brightness_percentage < 0) {
        brightness_percentage = 0;
    } else if (brightness_percentage > 100) {
        brightness_percentage = 100;
    }

    set_brightness(brightness_percentage, displays);
    return 0;
}

void set_brightness(double percentage, std::vector<Display> &displays) {
    if (percentage < 0) {
        percentage = 0;
    } else if (percentage > 100) {
        percentage = 100;
    }

    std::cout << "Current brightness: " << (int) (percentage) << '%' << std::endl;

    for (auto display : displays) {
        int value = display.min + (percentage/100 * (display.max - display.min));
        std::cout << "Display " << display.number << ": " << value << '%' << std::endl;

        uint8_t hi_byte = (uint8_t) (value >> 8);
        uint8_t lo_byte = (uint8_t) (value & 0xFF);
        ddca_set_non_table_vcp_value(display.handle, feature_code, hi_byte, lo_byte);
    }
}

// Gets the brightness of the given ddcutil display number.
// Throws if ddcutil fails to return a brightness percentage.
int get_brightness(Display &display) {
    DDCA_Non_Table_Vcp_Value value;
    ddca_get_non_table_vcp_value(display.handle, feature_code, &value);
    return value.sh << 8 | value.sl;
}

void print_brightness(std::vector<Display> displays)
{
    std::cout << "Current brightness" << std::endl;

    for (auto &display : displays) {
        std::cout << "Display " << display.number << ": " << get_brightness(display) << '%' << std::endl;
    }
}

std::vector<Display> get_displays() {
    std::vector<Display> displays;
    DDCA_Display_Info_List* info_list;
    ddca_get_display_info_list2(false, &info_list);

    for (int i = 0; i < info_list->ct; i++) {
        DDCA_Display_Info info = info_list->info[i];
        Display display = {
            info.dispno,
            1,
            100,
        };
        ddca_open_display2(info.dref, false, &display.handle);
        displays.emplace_back(display);
    }

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
    for (auto& item : *display_list) {
        toml::table disp_table = *item.as_table();
        int disp_num = disp_table["number"].as_integer()->get();
        int index = find_display(displays, disp_num);
        if (index == -1) {
            std::cerr << "Display number " << disp_num << " not found" << std::endl;
            exit(1);
        }
        displays[index].min = disp_table["min"].value_or(1);
        displays[index].max = disp_table["max"].value_or(100);

        disp_table.clear();
        continue;
    }

    display_list->clear();
    return displays;
}

int find_display(const std::vector<Display> &displays, int number) {
    for (int i = 0; i < displays.size(); i++) {
        if (displays[i].number == number) {
            return i;
        }
    }
    return -1;
}

void help() {
    std::cout << "Usage: brightness_sync [command] [value]\n"
    "Commands:\n"
    "   help                    prints this help page\n"
    "   up                      increases the brightness by 10%\n"
    "   down                    decreases the brightness by 10%\n"
    "   s, set VALUE            sets the brightness to the value\n"
    "   c, change VALUE         changes the brightness by the value\n"
    "   g, get VALUE            gets the brightness of all the monitors\n"
    << std::endl;
}
