#pragma once
#include <string>
#include <unistd.h>
#include <filesystem>
#include <pwd.h>
// #include <limits.h>  HOST_NAME_MAX
#include <sys/param.h>
#include <vector>
#include <memory>
#include <numeric>
#include "beautiful_prompt.hpp"


class UserNameModule : public BPModule {
private:
    std::string color_dark  = GREEN;
    std::string color_light = BLUE;
    std::string color_root = RED;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string username, hostname;
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL) {
            username = "[unknown user]";
        } else {
            username = std::string(pw->pw_name);
        }
        if (getuid() == 0) {
            username = "";  // TODO: move to settings
        }
        char hostname_chr[MAXHOSTNAMELEN + 1];
        if (gethostname(hostname_chr, MAXHOSTNAMELEN) != 0) {
            hostname = "[unknown hostname]";
        } else {
            hostname = std::string(hostname_chr);
        }

        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        if (getuid() == 0) active_color = RED;
        return Colorizer::paint(username + "@" + hostname, active_color, ctx.shell, cfg.use_colors);
    }
};


class SymbolModule : public BPModule {
private:
    std::string color_dark  = BLUE;
    std::string color_light = YELLOW;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string sym = "$";
        if (getuid() == 0) {
            return "#";
        }
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(sym, active_color, ctx.shell, cfg.use_colors);
    }
};


class PathModule : public BPModule {
private:
    std::string color_dark  = BLUE;
    std::string color_light = YELLOW;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::error_code ec;
        std::filesystem::path current_path = std::filesystem::current_path(ec);
        
        if (ec) return "[unknown path]";

        std::string path_str = current_path.string();
        const char* home_dir = std::getenv("HOME");

        if (home_dir != nullptr) {
            std::string home_str(home_dir);
            if (
                path_str.find(home_str) == 0
                && (
                    path_str.size() == home_str.size()
                    || (
                        path_str.size() > home_str.size()
                        && path_str.at(home_str.size()) == '/'
                    )
                )
            ) {
                path_str.replace(0, home_str.length(), "~");
            }
        }

        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        return Colorizer::paint(path_str, active_color, ctx.shell, cfg.use_colors);
    }
};


class DummyModule : public BPModule {
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        return "";
    }
};


class SpacerModule : public BPModule {
private:
    static inline uint8_t instances = 0;
    uint8_t current_instance;
    std::string color_dark  = THINWHITE;
    std::string color_light = THINBLACK;
    std::vector<std::unique_ptr<BPModule>> sub_modules;
public:
    template<typename... Args>
    SpacerModule(Args&&... args) {
        current_instance = instances;
        ++instances;
        (sub_modules.push_back(std::forward<Args>(args)), ...);
    }

    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string inner_content = "";
        std::string module_output = "";
        for (const auto& mod : sub_modules) {
            if (mod) {
                module_output = mod->render(ctx, cfg);
                if (!module_output.empty()) {
                    if (!inner_content.empty()) {
                        inner_content += " ";
                    }
                    inner_content += module_output;
                }
            }
        }

        if (inner_content.empty()) return "";

        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        if (current_instance == 0) {
            if (instances > 1) {
                return Colorizer::paint("┌─", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
             } else {
                return inner_content;
             }
        } else if (current_instance == instances - 1) {
            return "\n" + Colorizer::paint("└─", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
        } else {
            return "\n" + Colorizer::paint("│ ", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
        }
        return "";  /* Should never be reached */
    }
};


/*
# ┌─┬─┐
# │ │ │
# ├─┼─┤
# │ │ │
# └─┴─┘
*/
