#include <ddcutil_types.h>
#include <vector>

struct display {
    int number;
    int min;
    int max;
};

int get_brightness(int display_number);

int change_brightness(int change, std::vector<display> &displays);

void set_brightness(double percentage, std::vector<display> &displays);

void print_brightness(std::vector<display> displays);

std::vector<display> get_displays();

void help();
