#pragma once
#include <string>
#include <unistd.h>
#include <filesystem>
#include <pwd.h>
#include <limits.h>
#include "beautiful_prompt.hpp"


class UserNameModule : public BPModule {
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string username, hostname;
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL) {
            username = "";
        } else {
            username = std::string(pw->pw_name);
        }
        if (getuid() == 0) {
            username = "";  // TODO: move to settings
        }
        char hostname_chr[HOST_NAME_MAX];
        if (gethostname(hostname_chr, HOST_NAME_MAX) != 0) {
            hostname = "";
        } else {
            hostname = std::string(hostname_chr);
        }
        return username + "@" + hostname;
    }
};


class SymbolModule : public BPModule {
public:
    // const int default_color = 1;
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        if (getuid() == 0) {
            return "#";
        }
        return "$";
    }
};


class PathModule : public BPModule {
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

        return path_str;
    }
};