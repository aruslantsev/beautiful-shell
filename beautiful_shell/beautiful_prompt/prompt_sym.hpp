#pragma once
#include <string>
#include <unistd.h>
#include "beautiful_prompt.hpp"


class SymbolModule : public BPModule {
public:
    std::string render() const override {
        if (getuid() == 0) {
            return "#";
        }
        return "$";
    }
};
