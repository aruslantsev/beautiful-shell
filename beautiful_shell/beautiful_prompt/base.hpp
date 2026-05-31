#pragma once
#include <string>
#include <unistd.h>
#include <filesystem>
#include <pwd.h>
// #include <limits.h>  HOST_NAME_MAX
#include <sys/param.h>
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


class SpacerModule : public BPModule {
private:
    static inline u_int8_t instances = 0;
    u_int8_t current_instance;
    std::string color_dark  = THINWHITE;
    std::string color_light = THINBLACK;
public:
    SpacerModule() {
        current_instance = instances;
        ++instances;
    }

    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
        if (current_instance == 0) {
            if (instances > 1) {
                return Colorizer::paint("┌─", active_color, ctx.shell, cfg.use_colors);
             } else {
                return "";
             }
        } else if (current_instance == instances - 1) {
            return "\n" + Colorizer::paint("└─", active_color, ctx.shell, cfg.use_colors);
        } else {
            return "\n" + Colorizer::paint("│ ", active_color, ctx.shell, cfg.use_colors);
        }
        return "";
    }
};


/*
# ┌─┬─┐
# │ │ │
# ├─┼─┤
# │ │ │
# └─┴─┘
*/
