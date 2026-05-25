#pragma once

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


int get_user_info(struct prompt_data *data) {
    data->uid = geteuid();
    data->is_root = (data->uid == 0);

    struct passwd *pw = getpwuid(data->uid);
    if (pw == NULL) {
        data->username[0] = '\0';
        return -1;
    }
    strncpy(data->username, pw->pw_name, FIELD_LENGTH);
    data->username[FIELD_LENGTH - 1] = '\0';
    return 0;
}


int get_hostname(struct prompt_data *data) {

    if (gethostname(data->hostname, FIELD_LENGTH) != 0) {
        data->hostname[0] = '\0';
        return -1;
    }
    data->hostname[FIELD_LENGTH - 1] = '\0';
    return 0;
}


int get_cwd(struct prompt_data *data) {
    char *home = getenv("HOME");
    char cwd[FIELD_LENGTH];
    if (getcwd(cwd, FIELD_LENGTH) == NULL) {
        snprintf(data->cwd, FIELD_LENGTH, ".");
        return -1;
    }
    if (home != NULL) {
        size_t home_len = strlen(home);
        /* path starts with home */
        if (home_len > 1 && strncmp(cwd, home, home_len) == 0) {
            /* home and path have equal length */
            if (cwd[home_len] == '\0') {
                snprintf(data->cwd, FIELD_LENGTH, "~");
                return 0;
            /* path starts with home and next char is separator '/' */
            } else if (cwd[home_len] == '/') {
                snprintf(data->cwd, FIELD_LENGTH, "~%s", cwd + home_len);
                return 0;
            }
        }
    }
    strncpy(data->cwd, cwd, FIELD_LENGTH);
    data->cwd[FIELD_LENGTH - 1] = '\0';
    return 0;
}


int get_localtime(struct prompt_data *data) {
    time_t now;
    struct tm *local;
    time(&now);
    local = localtime(&now);
    return strftime(data->localtime, FIELD_LENGTH, "%Y-%m-%d %H:%M:%S", local) == 0;
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