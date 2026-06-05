#include <cstdlib>
#include "beautiful_prompt.hpp"


class EnvMonitorModule : public BPModule {
private:
    std::string color_dark  = YELLOW;
    std::string color_light = YELLOW;
public:
    std::string render(const BPContext& ctx, const BPSettings& cfg) const override {
        std::string result = "";

        const char* ssh_env = std::getenv("SSH_CONNECTION");
        bool is_ssh = (ssh_env != nullptr && std::string(ssh_env).length() > 0);
        
        if (is_ssh) {
            result += "SSH";
        }

        if (ctx.shlvl > 1) {
            if (!result.empty()) result += "|";
            result += "↳" + std::to_string(ctx.shlvl);
        }

        if (ctx.jobs_count > 0) {
            if (!result.empty()) result += "|";
            result += "&" + std::to_string(ctx.jobs_count);
        }

        const char* tmux_env = std::getenv("TMUX");
        if (tmux_env != nullptr && std::string(tmux_env).length() > 0) {
            if (!result.empty()) result += "|";
            result += "tmux";
        } else {
            const char* screen_env = std::getenv("STY");
            if (screen_env != nullptr && std::string(screen_env).length() > 0) {
                if (!result.empty()) result += "|";
                result += "screen";
            }
        }

        if (!result.empty()) {
            result = "[" + result + "]";
            std::string active_color = (cfg.color_theme == color_theme::DARK) ? color_dark : color_light;
            return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
        }

        return "";
    }
};