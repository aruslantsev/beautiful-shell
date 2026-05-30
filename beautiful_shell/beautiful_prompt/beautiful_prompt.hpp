#pragma once
#include <string>
#include <vector>
#include <memory>
#include "colors.hpp"


enum class shell {POSIX, BASH, ZSH};
enum class color_theme {DARK, LIGHT, CUSTOM};

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

/* From config files */
struct BPSettings {
    double              time_threshold_sec  = 1.0;
    bool                use_colors          = true;
    enum color_theme    color_theme         = color_theme::DARK;
};

/* From cmdline parameters */
struct BPContext {
    enum shell          shell               = shell::BASH;
    int                 exit_code           = 0;
    double              exec_time_sec       = 0.0;
};


class BPModule {
public:
    virtual ~BPModule() = default;
    virtual std::string render(const BPContext &ctx, const BPSettings &cfg) const = 0;  // TODO: why?
};


class PromptEngine {
private:
    std::vector<std::unique_ptr<BPModule>> modules;

public:
    void add_module(std::unique_ptr<BPModule> module) {
        modules.push_back(std::move(module));
    }

    std::string build_prompt(const BPContext &ctx, const BPSettings &cfg) const {
        std::string final_prompt, module_output;
        
        for (size_t i = 0; i < modules.size(); ++i) {
            module_output = modules[i]->render(ctx, cfg);
            if (module_output.size() > 0) {
                final_prompt += module_output;
                if (i < modules.size() - 1) {
                    final_prompt += " ";
                }
            }
        }

        final_prompt += " ";
        
        return final_prompt;
    }
};
