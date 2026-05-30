#pragma once
#include "beautiful_prompt.hpp"


class CMDStatusModule : public BPModule {
private:
    std::string color_err  = RED;
    std::string color_ok = GREEN;
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string ret = "";
        bool has_err = false;
        if (ctx.exit_code != 0) {
            ret += "[" + std::to_string(ctx.exit_code);
            has_err = true;
        }
        if (!ctx.pipe_status.empty()) {
            bool pipe_has_err = false;
            std::string pipe_str = "";
            for (size_t i = 0; i < ctx.pipe_status.size(); ++i) {
                if (ctx.pipe_status[i] != 0) pipe_has_err = true;
                pipe_str += std::to_string(ctx.pipe_status[i]);
                if (i < ctx.pipe_status.size() - 1) {
                    pipe_str += "|";
                }
            }
            if (pipe_has_err) {
                ret += ret.empty() ? "[" : "|";
                ret += pipe_str;
                has_err = true;
            }
        }
        if (ctx.exec_time_sec >= cfg.time_threshold_sec) {
            ret += ret.empty() ? "[" : "|";
            std::string execution_time = "";
            char buffer[1024];
            if (ctx.exec_time_sec < 10) {
                snprintf(buffer, sizeof(buffer), "%.1fs", ctx.exec_time_sec);
                ret += std::string(buffer);
            } else {
                int d = static_cast<int>(ctx.exec_time_sec) / 60 / 60 / 24;
	            int h = static_cast<int>(ctx.exec_time_sec) / 60 / 60 % 24;
	            int m = static_cast<int>(ctx.exec_time_sec) / 60 % 60;
	            int s = static_cast<int>(ctx.exec_time_sec) % 60;
                if (d > 0) {
                    snprintf(buffer, sizeof(buffer), "%dd", d);
                    ret += std::string(buffer);
                }
                if (h > 0 || d > 0) {
                    snprintf(buffer, sizeof(buffer), "%dh", h);
                    ret += std::string(buffer);
                }
                if (m > 0 || h > 0 || d > 0) {
                    snprintf(buffer, sizeof(buffer), "%dm", m);
                    ret += std::string(buffer);
                }
                snprintf(buffer, sizeof(buffer), "%ds", s);
                ret += std::string(buffer);
            }
        }
        if (!ret.empty()) {
            ret += "]";
        }
        std::string active_color = has_err ? color_err : color_ok;
        return Colorizer::paint(ret, active_color, ctx.shell, cfg.use_colors);
    }
};