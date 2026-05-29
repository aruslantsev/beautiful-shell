#pragma once
#include "beautiful_prompt.hpp"


class CMDStatusModule : public BPModule {
public:
    std::string render(const BPContext &ctx, const BPSettings &cfg) const override {
        std::string ret = "";
        if (ctx.exit_code != 0) {
            ret += "[" + std::to_string(ctx.exit_code);
        }
        if (ctx.exec_time_sec >= cfg.time_threshold_sec) {
            if (ret.size() == 0) {
                ret += "[";
            } else {
                ret += "|";
            }
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
        if (ret.size() > 0) {
            ret += "]";
        }
        return ret;
    }
};