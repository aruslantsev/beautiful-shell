#include <iostream>
#include "terminal.hpp"
#include "beautiful_prompt.hpp"
#include "base.hpp"

/*
int main(int argc, char *argv[]) {
    size_t rows = 0, cols = 0;
    getTermParams(rows, cols);
    std::cout << rows << " " << cols << std::endl;
    for (size_t argnum = 0; argnum < argc; argnum++) {
        std::cout << argv[argnum] << std::endl;
    }
    return 0;
}

*/

int main(int argc, char* argv[]) {
    PromptEngine engine;

    engine.add_module(std::make_unique<UserNameModule>());
    engine.add_module(std::make_unique<PathModule>());
    engine.add_module(std::make_unique<SymbolModule>());

    struct BPSettings cfg;
    struct BPContext ctx;

    std::cout << engine.build_prompt(ctx, cfg);

    return 0;
}


/*

TODO:
context: term size, term color, shell, exit code, pipestatus, etc.?
colors
settings: colors, module settings. implement colorize(context, settings)
default colors for each module
implement ... init bash/zsh/posix/...

*/