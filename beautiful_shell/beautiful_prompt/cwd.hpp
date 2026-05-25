#pragma once
#include <filesystem>
#include "beautiful_prompt.hpp"


class PathModule : public BPModule {
public:
    std::string render() const override {
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