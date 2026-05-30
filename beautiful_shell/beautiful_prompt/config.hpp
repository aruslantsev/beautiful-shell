#pragma once
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include "beautiful_prompt.hpp"

inline BPSettings load_settings() {
    BPSettings cfg;

    // (~/.config/beautiful_prompt/config
    const char* home_dir = std::getenv("HOME");
    if (!home_dir) return cfg;

    std::filesystem::path config_path = std::filesystem::path(home_dir) 
                                        / ".config" 
                                        / "beautiful_prompt" 
                                        / "config";

    if (!std::filesystem::exists(config_path)) {
        return cfg;
    }

    std::ifstream file(config_path);
    std::string line;
    
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty() || line[0] == '#') continue;

        size_t delimiter_pos = line.find('=');
        if (delimiter_pos == std::string::npos) continue;

        std::string key = line.substr(0, delimiter_pos);
        std::string value = line.substr(delimiter_pos + 1);

        if (key == "theme") {
            if (value == "light") cfg.color_theme = color_theme::LIGHT;
            else if (value == "custom") cfg.color_theme = color_theme::CUSTOM;
            else cfg.color_theme = color_theme::DARK;
        } 
        else if (key == "use_colors") {
            cfg.use_colors = (value == "true" || value == "1");
        } 
        else if (key == "time_threshold") {
            try {
                cfg.time_threshold_sec = std::stod(value);
            } catch (...) {
                // Ignore errors
            }
        }
    }

    return cfg;
}