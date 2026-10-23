#pragma once
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <git2.h>
#include "beautiful_shell.hpp"

struct Libgit2Runtime {
    Libgit2Runtime() { git_libgit2_init(); }
    ~Libgit2Runtime() { git_libgit2_shutdown(); }
    Libgit2Runtime(const Libgit2Runtime&) = delete;
    Libgit2Runtime& operator=(const Libgit2Runtime&) = delete;
};

struct GitRepoDeleter {
    void operator()(git_repository* repo) const {
        if (repo) git_repository_free(repo);
    }
};

struct GitStatusDeleter {
    void operator()(git_status_list* list) const {
        if (list) git_status_list_free(list);
    }
};

using GitRepoPtr = std::unique_ptr<git_repository, GitRepoDeleter>;
using GitStatusPtr = std::unique_ptr<git_status_list, GitStatusDeleter>;


class CondaModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = GREY;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        std::string result = "";
        const char* conda_env = std::getenv("CONDA_DEFAULT_ENV");
        if(conda_env != nullptr && std::string(conda_env).length() > 0) {
            result = "Conda: " + std::string(conda_env) + ".";
        }
        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};


class ESPIDFModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = GREY;
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        std::string result = "";
        const char* esp_env = std::getenv("ESP_IDF_VERSION");
        if(esp_env != nullptr && std::string(esp_env).length() > 0) {
            result = "ESP IDF: " + std::string(esp_env) + ".";
        }
        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};


class GitModule : public BSModule {
private:
    std::string color_dark  = GREY;
    std::string color_light = GREY;

    bool is_repository_too_large(const char* git_dir_path) const {
        try {
            std::filesystem::path index_path = std::filesystem::path(git_dir_path) / "index";
            if (std::filesystem::exists(index_path)) {
                if (std::filesystem::file_size(index_path) > 5 * 1024 * 1024) {
                    return true;
                }
            }
        } catch (...) {}
        return false;
    }
public:
    std::string render(const BSContext &ctx, const BSSettings &cfg) const override {
        Libgit2Runtime git_runtime;

        git_repository *raw_repo = nullptr;
        if (git_repository_open_ext(&raw_repo, ".", 0, nullptr) != 0) {
            return ""; // Not in repo
        }
        GitRepoPtr repo(raw_repo);

        std::string branch_name = "";
        git_reference *head = nullptr;

        // HEAD
        if (git_repository_head(&head, repo.get()) == 0) {
            // On branch -> name
            branch_name = git_reference_shorthand(head);
            git_reference_free(head);
        } else {
            // detached HEAD -> commit hash
            git_oid oid;
            if (git_reference_name_to_id(&oid, repo.get(), "HEAD") == 0) {
                char out[8];
                git_oid_tostr(out, sizeof(out), &oid);
                branch_name = std::string(out);
            }
        }

        if (branch_name.empty()) {
            return "";
        }

        std::string flags = "";
        const char* git_dir = git_repository_path(repo.get());

        if (git_dir != nullptr && is_repository_too_large(git_dir)) {
            // Big repo
            flags = "~"; 
        } else {
            // Fast status scan
            git_status_options opts;
            git_status_options_init(&opts, GIT_STATUS_OPTIONS_VERSION);
            opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
            // No submodules
            opts.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | 
                         GIT_STATUS_OPT_EXCLUDE_SUBMODULES | 
                         GIT_STATUS_OPT_DEFAULTS;

            git_status_list *raw_status = nullptr;
            if (git_status_list_new(&raw_status, repo.get(), &opts) == 0) {
                GitStatusPtr status_list(raw_status);
                size_t count = git_status_list_entrycount(status_list.get());
                bool has_staged = false;
                bool has_unstaged = false;
                bool has_untracked = false;

                for (size_t i = 0; i < count; ++i) {
                    const git_status_entry *entry = git_status_byindex(status_list.get(), i);
                    if (!entry) continue;
                    unsigned int s = entry->status;

                    // staged -> '+'
                    if (s & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | 
                             GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | 
                             GIT_STATUS_INDEX_TYPECHANGE)) {
                        has_staged = true;
                    }
                    // unstaged -> '*'
                    if (s & (GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | 
                             GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED)) {
                        has_unstaged = true;
                    }
                    // untracked -> '%'
                    if (s & GIT_STATUS_WT_NEW) {
                        has_untracked = true;
                    }
                }

                if (has_staged)    flags += "+";
                if (has_unstaged)  flags += "*";
                if (has_untracked) flags += (ctx.shell == shell::ZSH) ? "%%" : "%";
            }
        }
        if (!flags.empty()) {
            flags = " [" + flags + "]";
        }

        if (ctx.shell == shell::ZSH) {
            size_t pos = 0;
            while ((pos = branch_name.find("%", pos)) != std::string::npos) {
                branch_name.replace(pos, 1, "%%");
                pos += 2;
            }
        }

        std::string result = "Git: " + branch_name + flags + ".";
        std::string active_color = pick_theme_color(cfg, color_dark, color_light);
        return Colorizer::paint(result, active_color, ctx.shell, cfg.use_colors);
    }
};
