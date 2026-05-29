#pragma once
#include <string>
#include <vector>
#include <memory>


enum shell {POSIX, BASH, ZSH};
enum color_theme {DARK, LIGHT, CUSTOM};

/* From config files */
struct BPSettings {
    double              time_threshold_sec  = 1.0;
    bool                use_colors          = true;
    enum color_theme    color_theme         = DARK;
};

/* From cmdline parameters */
struct BPContext {
    enum shell          shell               = BASH;
    int                 exit_code           = 0;
    double              exec_time_sec       = 0.0;
};


class BPModule {
public:
    virtual ~BPModule() = default;
    // const int default_color = 0;
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
