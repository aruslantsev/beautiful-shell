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
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%.1fs", ctx.exec_time_sec);
            ret += std::string(buffer);
        }
        if (ret.size() > 0) {
            ret += "]";
        }
        return ret;
    }
};