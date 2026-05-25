#pragma once
#include <string>
#include <vector>
#include <memory>


enum SHELL {POSIX, BASH, ZSH};


class BPModule {
public:
    virtual ~BPModule() = default;
    virtual std::string render() const = 0;  // TODO: why?
};


class PromptEngine {
private:
    std::vector<std::unique_ptr<BPModule>> modules;

public:
    void add_module(std::unique_ptr<BPModule> module) {
        modules.push_back(std::move(module));
    }

    std::string build_prompt() const {
        std::string final_prompt;
        
        for (size_t i = 0; i < modules.size(); ++i) {
            final_prompt += modules[i]->render();
            
            if (i < modules.size() - 1) {
                final_prompt += " ";
            }
        }

        final_prompt += " ";
        
        return final_prompt;
    }
};
