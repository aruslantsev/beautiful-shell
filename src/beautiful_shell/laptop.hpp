#pragma once
#include <string>
#include <cstdlib>

#if defined(__linux__)
#include <fstream>
#include <filesystem>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined (__DragonFly__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include "beautiful_shell.hpp"

class BatteryModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;

public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        int percentage = -1;
        bool is_charging = false;
        bool is_not_charging = false;
        bool is_discharging = false;
        bool has_battery = false;

        if (!cfg.show_battery) return "";

#if defined(__linux__)
        std::string bat_dir = "/sys/class/power_supply/BAT0";
        if (!std::filesystem::exists(bat_dir)) {
            bat_dir = "/sys/class/power_supply/BAT1";
        }

        std::ifstream cap_file(bat_dir + "/capacity");
        if (cap_file >> percentage) {
            has_battery = true;
            
            std::ifstream stat_file(bat_dir + "/status");
            std::string status;
            if (std::getline(stat_file, status)) {
                if (status.find("Charging") != std::string::npos && status.find("Not") == std::string::npos) {
                    is_charging = true;
                } else if (status.find("Not charging") != std::string::npos || status.find("Full") != std::string::npos) {
                    is_not_charging = true;
                } else if (status.find("Discharging") != std::string::npos) {
                    is_discharging = true;
                }
            }
        }

#elif defined(__APPLE__)
        CFTypeRef ps_info = IOPSCopyPowerSourcesInfo();
        CFArrayRef ps_list = IOPSCopyPowerSourcesList(ps_info);

        if (ps_list && CFArrayGetCount(ps_list) > 0) {
            // First power source
            CFDictionaryRef p_source = IOPSGetPowerSourceDescription(ps_info, CFArrayGetValueAtIndex(ps_list, 0));
            if (p_source) {
                // Internal bat; not ups/stationary mac
                CFStringRef type = (CFStringRef)CFDictionaryGetValue(p_source, CFSTR(kIOPSTypeKey));
                if (type && CFStringCompare(type, CFSTR(kIOPSInternalBatteryType), 0) == kCFCompareEqualTo) {
                    
                    CFNumberRef cap = (CFNumberRef)CFDictionaryGetValue(p_source, CFSTR(kIOPSCurrentCapacityKey));
                    CFNumberRef max_cap = (CFNumberRef)CFDictionaryGetValue(p_source, CFSTR(kIOPSMaxCapacityKey));
                    int cur_val = 0, max_val = 100;
                    
                    if (cap) CFNumberGetValue(cap, kCFNumberIntType, &cur_val);
                    if (max_cap) CFNumberGetValue(max_cap, kCFNumberIntType, &max_val);
                    
                    if (max_val > 0) {
                        percentage = (cur_val * 100) / max_val;
                        has_battery = true;
                    }

                    CFStringRef state = (CFStringRef)CFDictionaryGetValue(p_source, CFSTR(kIOPSPowerSourceStateKey));
                    CFBooleanRef charging = (CFBooleanRef)CFDictionaryGetValue(p_source, CFSTR(kIOPSIsChargingKey));
                    bool mac_is_charging = false;
                    if (charging) {
                        mac_is_charging = CFBooleanGetValue(charging);
                    }

                    if (state) {
                        if (CFStringCompare(state, CFSTR(kIOPSBatteryPowerValue), 0) == kCFCompareEqualTo) {
                            is_discharging = true; // bat -> discharge
                        } else if (CFStringCompare(state, CFSTR(kIOPSACPowerValue), 0) == kCFCompareEqualTo) {
                            if (mac_is_charging) {
                                is_charging = true;     // ac, charge
                            } else {
                                is_not_charging = true; // ac, no charge
                            }
                        }
                    }
                }
            }
        }
        if (ps_list) CFRelease(ps_list);
        if (ps_info) CFRelease(ps_info);

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined (__DragonFly__)
        int units = 0;
        size_t units_len = sizeof(units);
        // Is bat presented (units > 0)
        if (sysctlbyname("hw.acpi.battery.units", &units, &units_len, NULL, 0) == 0 && units > 0) {
            int life = -1;
            size_t len = sizeof(life);
            
            if (sysctlbyname("hw.acpi.battery.life", &life, &len, NULL, 0) == 0 && life >= 0 && life <= 100) {
                percentage = life;
                has_battery = true;
                
                int state = -1;
                size_t state_len = sizeof(state);
                if (sysctlbyname("hw.acpi.battery.state", &state, &state_len, NULL, 0) == 0) {
                    // ACPI mapping:
                    if (state == 1) {
                        is_discharging = true;
                    } else if (state == 2) {
                        is_charging = true;
                    } else if (state == 0) {
                        is_not_charging = true;
                    }
                }
            }
        }
#endif
        if (!has_battery || percentage < 0) {
            return "";
        }

        std::string state_sym = "";
        if (is_charging) {
            state_sym = "+";
        } else if (is_not_charging) {
            state_sym = "=";
        } else if (is_discharging) {
            state_sym = "";
        }

        std::string result = "Bat: " + std::to_string(percentage) + "%" + state_sym + ".";
        
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        
        if ((is_charging || is_not_charging) && percentage > cfg.battery_hide_percent) return "";
        
        if (percentage <= cfg.battery_warn_percent && !is_charging && !is_not_charging) {
            active_color = RED;
        }

        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};