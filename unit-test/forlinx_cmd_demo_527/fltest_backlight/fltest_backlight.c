#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#define BASE_PATH "/sys/class/backlight/"

void print_usage() {
    fprintf(stderr, "用法: fltest_backlight get|set  [设备号] [亮度值]\n");
    fprintf(stderr, "示例:\n");
    fprintf(stderr, "  fltest_backlight get 0\n");
    fprintf(stderr, "  fltest_backlight set 0 100\n");
}

int get_max_brightness(const char *max_brightness_path) {
    FILE *file = fopen(max_brightness_path, "r");
    int max;
    if (fscanf(file, "%d", &max) != 1) {
        fprintf(stderr, "Unable to read maximum brightness value\n");
        fclose(file);
        exit(1);
    }
    fclose(file);
    return max;
}

int get_current_brightness(const char *brightness_path) {
    FILE *file = fopen(brightness_path, "r");
    int current;
    if (fscanf(file, "%d", &current) != 1) {
        fclose(file);
        exit(1);
    }
    fclose(file);
    return current;
}

void set_brightness(const char *brightness_path, const char *max_brightness_path, int new_value) {
    int max = get_max_brightness(max_brightness_path);
    if (new_value < 0 || new_value > max) {
        fprintf(stderr, "Error: Brightness value must be between 0 and%d\n", max);
        exit(1);
    }

    FILE *file = fopen(brightness_path, "w");

    if (fprintf(file, "%d", new_value) < 0) {
        fclose(file);
        exit(1);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }
	int device_num = atoi(argv[2]);	
	char brightness_path[PATH_MAX];
    char max_brightness_path[PATH_MAX];
    snprintf(brightness_path, sizeof(brightness_path), BASE_PATH "backlight%d/brightness", device_num);
    snprintf(max_brightness_path, sizeof(max_brightness_path), BASE_PATH "backlight%d/max_brightness", device_num);

    if (strcmp(argv[1], "get") == 0) {
        int current = get_current_brightness(brightness_path);
        printf("Current brightness: %d\n", current);
    } else if (strcmp(argv[1], "set") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Setting brightness requires specifying a value\n");
            print_usage();
            return 1;
        }

        char *endptr;
        long value = strtol(argv[3], &endptr, 10);
        if (*endptr != '\0' || value < 0) {
            fprintf(stderr, "Error: Invalid brightness value (must be a non negative integer)\n");
            return 1;
        }

        set_brightness(brightness_path,max_brightness_path,value);
        printf("The brightness has been set to: %ld\n", value);
    } else {
        print_usage();
        return 1;
    }

    return 0;
}
