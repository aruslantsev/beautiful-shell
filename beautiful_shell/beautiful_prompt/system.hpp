#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <string>
#include <cstdlib>
#include <sys/sysinfo.h>
#include "beautiful_prompt.hpp"


class DateTimeModule : public BPModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        // Format: YYYY-MM-DD HH:MM:SS
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(oss.str(), active_color, ctx.shell, cfg.use_colors);
    }
};


class LoadAVGModule : public BPModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        double load[3];
        std::ostringstream out;
        out << "LoadAvg: ";
        if (getloadavg(load, 3) != -1) {
            out << std::fixed << std::setprecision(2) << load[0] << ", ";
            out << std::fixed << std::setprecision(2) << load[1] << ", ";
            out << std::fixed << std::setprecision(2) << load[2] << ".";
            std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
            return Colorizer::paint(out.str(), active_color, ctx.shell, cfg.use_colors);
        }
        return "";
    }
};


class RAMModule : public BPModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = BLACK;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        struct sysinfo si;
        std::ostringstream out;
        if (sysinfo(&si) == 0) {
            // Multiplier to convert to bytes (mem_unit is often 1 or page size)
            unsigned long long multiplier = si.mem_unit;

            unsigned long long total_ram = si.totalram * multiplier;
            unsigned long long free_ram = si.freeram * multiplier;
            unsigned long long used_ram = total_ram - free_ram;

            unsigned long long total_swap = si.totalswap * multiplier;
            unsigned long long free_swap = si.freeswap * multiplier;
            unsigned long long used_swap = total_swap - free_swap;


            std::string unit;
            int divisor = 1;
            if (total_ram <= 1024 * 1024 * 1024) {
                unit = "MB";
                divisor = 1024 * 1024;
            } else {
                unit = "GB";
                divisor = 1024 * 1024 * 1024;
            }
            out << "Mem: " << used_ram / divisor << "/" << total_ram / divisor << unit << ", ";
            out << "Swap: " << used_swap / divisor << "/" << total_swap / divisor << unit << ".";
            std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
            return Colorizer::paint(out.str(), active_color, ctx.shell, cfg.use_colors);
        }
        return "";
    }
};
