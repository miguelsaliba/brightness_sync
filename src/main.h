#include <vector>
#include "config.h"

int get_brightness(int display_number);

int change_brightness(int change, std::vector<Display> &displays);

void set_brightness(double percentage, std::vector<Display> &displays);

void print_brightness(std::vector<Display> displays);

std::vector<Display> get_displays();

void help();
