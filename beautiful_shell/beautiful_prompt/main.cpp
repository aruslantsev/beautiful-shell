#include <iostream>
#include "beautiful_prompt.hpp"
#include "base.hpp"
#include "cmd.hpp"
#include "system.hpp"
#include "status.hpp"
#include "config.hpp"


BPContext parse_args(int argc, char* argv[]) {
    BPContext ctx;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--shell" && i + 1 < argc) {
            std::string sh = argv[++i];
            if (sh == "bash") {
                ctx.shell = shell::BASH;
            } else if (sh == "zsh") {
                ctx.shell = shell::ZSH;
            } else {
                ctx.shell = shell::POSIX;
            }
        }
        else if (arg == "--time" && i + 1 < argc) ctx.exec_time_sec = std::stod(argv[++i]);
        else if (arg == "--pipestatus" && i + 1 < argc) {
            std::string pipe_str = argv[++i];
            std::stringstream ss(pipe_str);
            std::string code;
            while (ss >> code) {
                try {
                    ctx.pipe_status.push_back(std::stoi(code));
                } catch (...) {}
            }
        }
        else if (arg == "--jobs" && i + 1 < argc) {
            try {
                ctx.jobs_count = std::stoi(argv[++i]);
            } catch (...) {
                ctx.jobs_count = 0;
            }
        }
        else if (arg == "--shlvl" && i + 1 < argc) ctx.shlvl = std::stoi(argv[++i]);
    }
    return ctx;
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && std::string(argv[1]) == "init") {
        std::string target_shell = argv[2];

        if (target_shell == "bash") {
            std::cout << "bp_start_time=\"\";\n"
                    << "beautiful_prompt_start_time() {\n"
                    << "    [ -z \"$bp_start_time\" ] && bp_start_time=${EPOCHREALTIME:-$(date +%s.%N)};\n"
                    << "};\n"
                    << "beautiful_prompt_set_ps1() {\n"
                    << "    local bp_pipe=(\"${PIPESTATUS[@]}\");\n"
                    << "    local end_time=${EPOCHREALTIME:-$(date +%s.%N)};\n"
                    << "    local exec_time=0;\n"
                    << "    if [ -n \"$bp_start_time\" ]; then\n"
                    << "        exec_time=$(awk \"BEGIN {print $end_time - $bp_start_time}\" 2>/dev/null || echo 0);\n"
                    << "    fi;\n"
                    << "    local env_jobs=$(jobs | wc -l);\n"
                    << "    PS1=\"$(beautiful_prompt --shell bash --pipestatus \"${bp_pipe[*]}\" --time $exec_time --jobs \"$env_jobs\" --shlvl \"$SHLVL\")\";\n"
                    << "    bp_start_time=\"\";\n"
                    << "};\n"
                    << "trap 'beautiful_prompt_start_time' DEBUG;\n"
                    << "PROMPT_COMMAND=beautiful_prompt_set_ps1;\n";
            return 0;
        } 
        else if (target_shell == "zsh") {
            std::cout << "zmodload zsh/datetime 2>/dev/null\n"
                    << "bp_start_time=\"\";\n"
                    << "beautiful_prompt_preexec() { bp_start_time=$EPOCHREALTIME }\n"
                    << "beautiful_prompt_precmd() {\n"
                    << "    local bp_pipe=(\"${pipestatus[@]}\");\n"
                    << "    local exec_time=0;\n"
                    << "    if [ -n \"$bp_start_time\" ]; then\n"
                    << "        exec_time=$(( EPOCHREALTIME - bp_start_time ))\n"
                    << "    fi;\n"
                    << "    local env_jobs=${#jobstates};\n"
                    << "    PROMPT=\"$(beautiful_prompt --shell zsh --pipestatus \"${bp_pipe[*]}\" --time $exec_time --jobs \"$env_jobs\" --shlvl \"$SHLVL\")\";\n"
                    << "    bp_start_time=\"\"\n"
                    << "}\n"
                    << "autoload -Uz add-zsh-hook\n"
                    << "add-zsh-hook preexec beautiful_prompt_preexec\n"
                    << "add-zsh-hook precmd beautiful_prompt_precmd\n";
            return 0;
        }
        else {
            std::cout << "PS1='$(beautiful_prompt --shell posix --pipestatus \"$?\") '\n";
            return 0;
        }
    }

    struct BPSettings cfg = load_settings();
    struct BPContext ctx = parse_args(argc, argv);

    PromptEngine engine;

    engine.add_module(std::make_unique<SpacerModule>());
    engine.add_module(std::make_unique<CMDStatusModule>());
    engine.add_module(std::make_unique<DateTimeModule>());
    engine.add_module(std::make_unique<LoadAVGModule>());
    engine.add_module(std::make_unique<RAMModule>());
    engine.add_module(std::make_unique<SpacerModule>());
    engine.add_module(std::make_unique<EnvMonitorModule>());
    engine.add_module(std::make_unique<UserNameModule>());
    engine.add_module(std::make_unique<PathModule>());
    engine.add_module(std::make_unique<SymbolModule>());

    std::cout << engine.build_prompt(ctx, cfg);

    return 0;
}


/*
TODO: ctx: term size
cfg: colors, module settings
*/