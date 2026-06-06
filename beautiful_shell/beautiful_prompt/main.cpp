#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <toml++/toml.hpp>
#include "beautiful_prompt.hpp"
#include "base.hpp"
#include "cmd.hpp"
#include "system.hpp"
#include "status.hpp"
#include "devtools.hpp"
#include "laptop.hpp"


inline BPSettings load_settings() {
    BPSettings cfg;

    const char* home_dir = std::getenv("HOME");
    if (!home_dir) return cfg;

    std::filesystem::path config_path = std::filesystem::path(home_dir) 
                                        / ".config" 
                                        / "beautiful_prompt" 
                                        / "config";

    if (!std::filesystem::exists(config_path)) {
        return cfg;
    }

    try {
        toml::table tbl = toml::parse_file(config_path.string());

        if (auto prompt = tbl["prompt"].as_table()) {
            
            if (auto theme_node = (*prompt)["theme"].as_string()) {
                std::string theme_val = std::string(theme_node->get());
                if (theme_val == "light") cfg.color_theme = color_theme::LIGHT;
                else if (theme_val == "dark") cfg.color_theme = color_theme::DARK;
                else cfg.color_theme = color_theme::CUSTOM;
            }

            cfg.use_colors = (*prompt)["use_colors"].value_or(cfg.use_colors);
            cfg.show_root_username = (*prompt)["show_root_username"].value_or(cfg.show_root_username);
            if (auto node = (*prompt)["time_threshold"]) {
                if (auto f = node.as_floating_point()) {
                    cfg.time_threshold_sec = f->get();
                } else if (auto i = node.as_integer()) {
                    cfg.time_threshold_sec = static_cast<double>(i->get());
                }
            }
        }

        if (auto shell = tbl["shell"].as_table()) {
            cfg.run_ssh_agent = (*shell)["run_ssh_agent"].value_or(cfg.run_ssh_agent);
            cfg.save_history  = (*shell)["save_history"].value_or(cfg.save_history);
            cfg.conda_init    = (*shell)["conda_init"].value_or(cfg.conda_init);
        }

        if (auto battery = tbl["battery"].as_table()) {
            cfg.show_battery = (*battery)["show_battery"].value_or(cfg.show_battery);
            cfg.battery_warn_percent = (*battery)["battery_warn_percent"].value_or(cfg.battery_warn_percent);
            cfg.battery_hide_percent = (*battery)["battery_hide_percent"].value_or(cfg.battery_hide_percent);
        }
    } catch (const toml::parse_error& err) {}
    return cfg;
}


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
    struct BPSettings cfg = load_settings();
    struct BPContext ctx = parse_args(argc, argv);
    if (argc >= 3 && std::string(argv[1]) == "init") {
        std::string target_shell = argv[2];

        std::cout << "_BS_RUN_SSH_AGENT=" << (cfg.run_ssh_agent ? "1" : "0") << "\n";
        std::cout << "_BS_SAVE_HISTORY=" << (cfg.save_history ? "1" : "0") << "\n";
        std::cout << "_BS_CONDA_INIT=" << (cfg.conda_init ? "1" : "0") << "\n";

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

    PromptEngine engine{
        std::make_unique<SpacerModule>(
            std::make_unique<CMDStatusModule>(),
            std::make_unique<DateTimeModule>(),
            std::make_unique<LoadAVGModule>(),
            std::make_unique<RAMModule>(),
            std::make_unique<BatteryModule>()
        ),
        std::make_unique<SpacerModule>(
            std::make_unique<GitModule>(),
            std::make_unique<CondaModule>(),
            std::make_unique<ESPIDFModule>()
        ),
        std::make_unique<SpacerModule>(
            std::make_unique<EnvMonitorModule>(),
            std::make_unique<UserNameModule>(),
            std::make_unique<PathModule>(),
            std::make_unique<SymbolModule>()
        )
    };
    std::cout << set_window_title(ctx.shell) << engine.build_prompt(ctx, cfg);

    return 0;
}
