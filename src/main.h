#include <ddcutil_types.h>
#include <vector>

struct Display {
    int number;
    int min;
    int max;
    DDCA_Display_Handle handle;
};

int get_brightness(Display &display);

int change_brightness(int change, std::vector<Display> &displays);

void set_brightness(double percentage, std::vector<Display> &displays);

void print_brightness(std::vector<Display> displays);

std::vector<Display> get_displays();

int find_display(const std::vector<Display> &displays, int number);

void help();
