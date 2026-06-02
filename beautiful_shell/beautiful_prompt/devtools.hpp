#pragma once
#include <cstdlib>
#include "beautiful_prompt.hpp"


class CondaModule : public BPModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = GREY;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string result = "";
        const char* conda_env = std::getenv("CONDA_DEFAULT_ENV");
        if(conda_env != nullptr && std::string(conda_env).length() > 0) {
            result = "Conda: " + std::string(conda_env) + ".";
        }
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};


class ESPIDFModule : public BPModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = GREY;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string result = "";
        const char* esp_env = std::getenv("ESP_IDF_VERSION");
        if(esp_env != nullptr && std::string(esp_env).length() > 0) {
            result = "ESP IDF: " + std::string(esp_env) + ".";
        }
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};