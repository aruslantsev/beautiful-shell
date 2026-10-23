#pragma once
#include <string>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>
#include <pwd.h>
#include <sys/param.h>
#include <vector>
#include <memory>
#include "beautiful_shell.hpp"


enum class row_role { First, Middle, Last };

inline std::string get_username() {
    struct passwd *pw = getpwuid(geteuid());
    if (pw == NULL) return "[unknown user]";
    return std::string(pw->pw_name);
}

inline std::string get_hostname() {
    char hostname_chr[MAXHOSTNAMELEN + 1];
    if (gethostname(hostname_chr, sizeof(hostname_chr)) != 0) {
        return "[unknown hostname]";
    }
    hostname_chr[sizeof(hostname_chr) - 1] = '\0';
    return std::string(hostname_chr);
}

inline std::string collapse_home(std::string path_str) {
    const char* home_dir = std::getenv("HOME");
    if (home_dir == nullptr) return path_str;

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
    return path_str;
}

inline std::string current_path_display() {
    std::error_code ec;
    std::filesystem::path current_path = std::filesystem::current_path(ec);
    if (ec) return "[unknown path]";
    return collapse_home(current_path.string());
}


class UserNameModule : public BSModule {
private:
    std::string color_dark  = GREEN;
    std::string color_light = BLUE;
    std::string color_root = RED;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        uid_t uid = geteuid();
        std::string username = get_username();
        if (uid == 0 && !cfg.show_root_username) {
            username = "";
        }
        std::string hostname = get_hostname();

        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        if (uid == 0) active_color = color_root;
        return Colorizer::paint(username + "@" + hostname, active_color, ctx.shell, cfg.use_colors);
    }
};


class SymbolModule : public BSModule {
private:
    std::string color_dark  = BLUE;
    std::string color_light = YELLOW;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        std::string sym = "$";
        if (geteuid() == 0) {
            sym = "#";
        }
        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        return Colorizer::paint(sym, active_color, ctx.shell, cfg.use_colors);
    }
};


class PathModule : public BSModule {
private:
    std::string color_dark  = BLUE;
    std::string color_light = YELLOW;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        return Colorizer::paint(current_path_display(), active_color, ctx.shell, cfg.use_colors);
    }
};


class SpacerModule : public BSModule {
private:
    row_role role;
    std::string color_dark  = THINWHITE;
    std::string color_light = THINBLACK;
    std::vector<std::unique_ptr<BSModule>> sub_modules;
public:
    template<typename... Args>
    SpacerModule(row_role role, Args&&... args) : role(role) {
        (sub_modules.push_back(std::forward<Args>(args)), ...);
    }

    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
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

        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        switch (role) {
            case row_role::First:
                return Colorizer::paint("┌─", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
            case row_role::Last:
                return "\n" + Colorizer::paint("└─", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
            case row_role::Middle:
            default:
                return "\n" + Colorizer::paint("│ ", active_color, ctx.shell, cfg.use_colors) + " " + inner_content;
        }
    }
};


/*
# ┌─┬─┐
# │ │ │
# ├─┼─┤
# │ │ │
# └─┴─┘
*/

inline std::string set_window_title(shell s) {
    std::string title_text = get_username() + "@" + get_hostname() + ":" + current_path_display();
    switch (s) {
        case shell::BASH:
            return "\001\033]0;" + title_text + "\007\002";
        case shell::ZSH:
            return "%{\033]0;" + title_text + "\007%}";

        case shell::POSIX:
        default:
            return "\033]0;" + title_text + "\007";
    }
}
