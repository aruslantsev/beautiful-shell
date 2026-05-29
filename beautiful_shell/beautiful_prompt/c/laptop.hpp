#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>


#define FIELD_LENGTH    255

enum bat_status {CHARGING = 0, DISCHARGING = 1, MISSING = -1};

struct prompt_data {
    uid_t               uid;
    bool                is_root;
    char                username[FIELD_LENGTH];
    char                hostname[FIELD_LENGTH];
    char                cwd[FIELD_LENGTH];
    char                localtime[FIELD_LENGTH];
    int8_t              return_code;
    int32_t             execution_time;
    int8_t              pipe_status;
    int32_t             num_jobs;
    int8_t              shlvl;
    enum bat_status     bat_status;
    uint8_t             bat_percent;
};

int get_battery_info(struct prompt_data *data) {
    FILE *status;
    FILE *capacity;
    char buffer[FIELD_LENGTH];
    status = fopen("/sys/class/power_supply/BAT0/status", "r");
    if (status != NULL) {
        if (fgets(buffer, FIELD_LENGTH, status) == NULL) {
            data->bat_status = MISSING;
            return -1;
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strcmp(buffer, "Discharging") == 0) {
            data->bat_status = DISCHARGING;
        } else {
            data->bat_status = CHARGING;
        }
        capacity = fopen("/sys/class/power_supply/BAT0/capacity", "r");
        data->bat_percent = 0;
        if (capacity != NULL) {
            if (fscanf(capacity, "%hhd", &(data->bat_percent)) != 1) {
                return -1;
            }
        } else {
            data->bat_percent = 0;
        }
    } else {
        data->bat_status = MISSING;
    }
    return 0;
}