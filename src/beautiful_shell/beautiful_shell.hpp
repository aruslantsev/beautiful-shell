#pragma once
#include <string>
#include <vector>
#include <memory>
#include "colors.hpp"


enum class shell {POSIX, BASH, ZSH};
enum class color_theme {DARK, LIGHT, CUSTOM};

/* From config files */
struct BSSettings {
    double              time_threshold_sec      = 1.0;
    bool                use_colors              = true;
    enum color_theme    color_theme             = color_theme::DARK;

    bool                show_root_username      = false;

    bool                run_ssh_agent           = true;
    bool                save_history            = false;
    bool                conda_init              = true;

    bool                show_battery            = true;
    int                 battery_warn_percent    = 20;
    int                 battery_hide_percent    = 80;
};

/* From cmdline parameters */
struct BSContext {
    enum shell          shell               = shell::POSIX;
    std::vector<int>    pipe_status;
    double              exec_time_sec       = 0.0;
    int                 jobs_count          = 0;
    int                 shlvl               = 1;
};


class BSModule {
public:
    virtual ~BSModule() = default;
    virtual std::string render(const BSContext &ctx, const BSSettings &cfg) const = 0;
};


class PromptEngine {
private:
    std::vector<std::unique_ptr<BSModule>> modules;
public:
    PromptEngine() = default;
    template<typename... Args>
    PromptEngine(Args&&... args) {
        (modules.push_back(std::forward<Args>(args)), ...);
    }

    std::string build_prompt(const BSContext &ctx, const BSSettings &cfg) const {
        std::string final_prompt;
        

        std::vector<std::string> active_outputs;
        for (const auto& mod : modules) {
            if (mod) {
                std::string out = mod->render(ctx, cfg);
                if (!out.empty()) {
                    active_outputs.push_back(std::move(out));
                }
            }
        }

        for (size_t i = 0; i < active_outputs.size(); ++i) {
            final_prompt += active_outputs[i];
            if (i < active_outputs.size() - 1) {
                final_prompt += " ";
            }
        }

        if (!final_prompt.empty()) final_prompt += " ";
        
        return final_prompt;
    }
};


class Colorizer {
public:
    static std::string paint(const std::string& text, const std::string& ansi_code, shell s, bool use_colors) {
        if (!use_colors || ansi_code.empty() || text.empty()) return text;
        std::string open_seq, close_seq;

        switch (s) {
            case shell::BASH:
                open_seq = "\001\033[" + ansi_code + "m\002";
                close_seq = "\001\033[0m\002";
                break;
            case shell::ZSH:
                open_seq = "%{\033[" + ansi_code + "m%}";
                close_seq = "%{\033[0m%}";
                break;
            default: // POSIX / default
                open_seq = "\033[" + ansi_code + "m";
                close_seq = "\033[0m";
                break;
        }
        return open_seq + text + close_seq;
    }
};
