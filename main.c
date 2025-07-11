#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "hidapi.h"

#define MAX_STR 255
#define LUXAFOR_VENDOR_ID 0x04d8
#define LUXAFOR_PRODUCT_ID 0xf372

typedef enum {
    CMD_SOLID = 1,
    CMD_FADE = 2,
    CMD_STROBE = 3,
    CMD_WAVE = 4,
    CMD_PATTERN = 6
} COMMAND_MODE;

typedef enum {
    LED_ALL = 0xFF,
    LED_FRONT = 0x41,
    LED_BACK = 0x42
} LED_TARGET;

typedef enum {
    RED,
    GREEN,
    BLUE,
    WHITE,
    BLACK,
    OFF,
    YELLOW,
    MAGENTA,
    CYAN,
    ORANGE,
    PURPLE,
    PINK
} SAMPLE_COLOR;

typedef struct {
    SAMPLE_COLOR color;
    const char *name;
    const char *rgbValue;
} s_Color;

static s_Color colorMap[] = {
    { RED, "red", "0xff0000" },
    { GREEN, "green", "0x00ff00" },
    { BLUE, "blue", "0x0000ff" },
    { WHITE, "white", "0xffffff" },
    { BLACK, "black", "0x000000" },
    { OFF, "off", "0x000000" },
    { YELLOW, "yellow", "0xffff00" },
    { MAGENTA, "magenta", "0xff00ff" },
    { CYAN, "cyan", "0x00ffff" },
    { ORANGE, "orange", "0xffa500" },
    { PURPLE, "purple", "0x800080" },
    { PINK, "pink", "0xffc0cb" },
};

void print_usage(const char *prog_name) {
    printf("Usage: %s [command] [options]\n", prog_name);
    printf("\nSimple color commands:\n");
    printf("  %s COLOR              # COLOR can be:\n", prog_name);
    printf("                        # red, green, blue, yellow, magenta, cyan,\n");
    printf("                        # orange, purple, pink, white, black, off\n");
    printf("  %s 0xRRGGBB           # Custom hex color\n", prog_name);
    printf("\nAdvanced commands:\n");
    printf("  %s fade --color COLOR [--speed SPEED] [--led LED]\n", prog_name);
    printf("  %s strobe --color COLOR [--speed SPEED] [--repeat COUNT] [--led LED]\n", prog_name);
    printf("  %s wave --type TYPE --color COLOR [--speed SPEED] [--repeat COUNT]\n", prog_name);
    printf("  %s pattern --id ID [--repeat COUNT]\n", prog_name);
    printf("\nOptions:\n");
    printf("  --color COLOR    Color name or hex value (0xRRGGBB)\n");
    printf("  --led LED        Target LED: all (default), front, back\n");
    printf("  --speed SPEED    Effect speed (0-255)\n");
    printf("  --repeat COUNT   Repeat count (0-255, 0=infinite)\n");
    printf("  --type TYPE      Wave type (1-5)\n");
    printf("  --id ID          Pattern ID (1-8)\n");
    printf("\nExamples:\n");
    printf("  %s off                           # Turn off the device\n", prog_name);
    printf("  %s red                           # Solid red\n", prog_name);
    printf("  %s fade --color blue --speed 20  # Fade to blue\n", prog_name);
    printf("  %s pattern --id 7 --repeat 3     # Run pattern 7 three times\n", prog_name);
}

int parse_color(const char *color_str, unsigned char *rgb) {
    int res = sscanf(color_str, "0x%02hhX%02hhX%02hhX", rgb, rgb + 1, rgb + 2);
    
    if (res != 3) {
        res = sscanf(color_str, "#%02hhX%02hhX%02hhX", rgb, rgb + 1, rgb + 2);
    }
    
    if (res != 3) {
        for (int i = 0; i < sizeof(colorMap) / sizeof(colorMap[0]); i++) {
            if (strcasecmp(color_str, colorMap[i].name) == 0) {
                sscanf(colorMap[i].rgbValue, "0x%02hhX%02hhX%02hhX", rgb, rgb + 1, rgb + 2);
                return 1;
            }
        }
        return 0;
    }
    
    return 1;
}

int parse_led_target(const char *led_str) {
    if (strcasecmp(led_str, "all") == 0) return LED_ALL;
    if (strcasecmp(led_str, "front") == 0) return LED_FRONT;
    if (strcasecmp(led_str, "back") == 0) return LED_BACK;
    return -1;
}

int send_command(hid_device *handle, unsigned char *buf, size_t size) {
    int res = hid_write(handle, buf, size);
    if (res == -1) {
        printf("* ERROR:\tUnable to write to USB device.\n");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        exit(1);
    }
    
    unsigned char buf[9] = {0};
    unsigned char rgb[3] = {0};
    int command_mode = CMD_SOLID;
    int led_target = LED_ALL;
    int speed = 20;
    int repeat = 0;
    int wave_type = 1;
    int pattern_id = 1;
    
    // Check for simple color command
    if (argc == 2 && argv[1][0] != '-') {
        if (parse_color(argv[1], rgb)) {
            // Simple solid color command
            buf[0] = 0;
            buf[1] = CMD_SOLID;
            buf[2] = LED_ALL;
            buf[3] = rgb[0];
            buf[4] = rgb[1];
            buf[5] = rgb[2];
        } else {
            printf("* ERROR:\tUnknown color: %s\n", argv[1]);
            exit(1);
        }
    } else {
        // Parse advanced commands
        static struct option long_options[] = {
            {"color", required_argument, 0, 'c'},
            {"led", required_argument, 0, 'l'},
            {"speed", required_argument, 0, 's'},
            {"repeat", required_argument, 0, 'r'},
            {"type", required_argument, 0, 't'},
            {"id", required_argument, 0, 'i'},
            {"help", no_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        
        // Determine command type
        if (strcmp(argv[1], "fade") == 0) {
            command_mode = CMD_FADE;
        } else if (strcmp(argv[1], "strobe") == 0) {
            command_mode = CMD_STROBE;
        } else if (strcmp(argv[1], "wave") == 0) {
            command_mode = CMD_WAVE;
        } else if (strcmp(argv[1], "pattern") == 0) {
            command_mode = CMD_PATTERN;
        } else {
            printf("* ERROR:\tUnknown command: %s\n", argv[1]);
            print_usage(argv[0]);
            exit(1);
        }
        
        // Parse options
        int opt;
        int option_index = 0;
        optind = 2;  // Skip command name
        
        while ((opt = getopt_long(argc, argv, "c:l:s:r:t:i:h", long_options, &option_index)) != -1) {
            switch (opt) {
                case 'c':
                    if (!parse_color(optarg, rgb)) {
                        printf("* ERROR:\tInvalid color: %s\n", optarg);
                        exit(1);
                    }
                    break;
                case 'l':
                    led_target = parse_led_target(optarg);
                    if (led_target == -1) {
                        printf("* ERROR:\tInvalid LED target: %s\n", optarg);
                        exit(1);
                    }
                    break;
                case 's':
                    speed = atoi(optarg);
                    if (speed < 0 || speed > 255) {
                        printf("* ERROR:\tSpeed must be 0-255\n");
                        exit(1);
                    }
                    break;
                case 'r':
                    repeat = atoi(optarg);
                    if (repeat < 0 || repeat > 255) {
                        printf("* ERROR:\tRepeat must be 0-255\n");
                        exit(1);
                    }
                    break;
                case 't':
                    wave_type = atoi(optarg);
                    if (wave_type < 1 || wave_type > 5) {
                        printf("* ERROR:\tWave type must be 1-5\n");
                        exit(1);
                    }
                    break;
                case 'i':
                    pattern_id = atoi(optarg);
                    if (pattern_id < 1 || pattern_id > 8) {
                        printf("* ERROR:\tPattern ID must be 1-8\n");
                        exit(1);
                    }
                    break;
                case 'h':
                    print_usage(argv[0]);
                    exit(0);
                default:
                    print_usage(argv[0]);
                    exit(1);
            }
        }
        
        // Build command buffer based on mode
        buf[0] = 0;  // Report number
        buf[1] = command_mode;
        
        switch (command_mode) {
            case CMD_FADE:
                buf[2] = led_target;
                buf[3] = rgb[0];
                buf[4] = rgb[1];
                buf[5] = rgb[2];
                buf[6] = speed;
                break;
                
            case CMD_STROBE:
                buf[2] = led_target;
                buf[3] = rgb[0];
                buf[4] = rgb[1];
                buf[5] = rgb[2];
                buf[6] = speed;
                buf[8] = repeat;
                break;
                
            case CMD_WAVE:
                buf[2] = wave_type;
                buf[3] = rgb[0];
                buf[4] = rgb[1];
                buf[5] = rgb[2];
                buf[7] = repeat;
                buf[8] = speed;
                break;
                
            case CMD_PATTERN:
                buf[2] = pattern_id;
                buf[3] = repeat;
                break;
        }
    }
    
    // Initialize HID
    int res = hid_init();
    if (res == -1) {
        printf("* ERROR:\tFailed to initialize HID library.\n");
        exit(1);
    }
    
    // Open device
    hid_device *handle = hid_open(LUXAFOR_VENDOR_ID, LUXAFOR_PRODUCT_ID, NULL);
    if (handle == NULL) {
        printf("* ERROR:\tUnable to open Luxafor device. Is it plugged in?\n");
        hid_exit();
        exit(1);
    }
    
    // Send command
    if (send_command(handle, buf, sizeof(buf)) == -1) {
        hid_close(handle);
        hid_exit();
        exit(1);
    }
    
    // Cleanup
    hid_close(handle);
    hid_exit();
    
    return 0;
}