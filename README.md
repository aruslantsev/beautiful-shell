# Beautiful Shell

`beautiful_shell` is a lightweight, fast, and cross-platform (Linux, macOS, BSD) command-line prompt and shell management ecosystem written in C++17. It creates informative, multi-line status layouts using native C++ processing, backed by a modular shell script bootloader designed to maximize shell environment organization.


## Features

* Modular Architecture: Easily add, remove, or rearrange informational blocks (`BSModule`) to fit your workflow.
* Smart Layouts (`SpacerModule`): Structures your prompt into multiple rows using clear box-drawing characters (`┌─`, `│ `, `└─`).
* High Performance: Built with native C++ and statically linked with `libgit2` to fetch Git repository statuses instantly without spawning heavy external CLI processes.
* True Cross-Platform Support: Accurately parses system metrics (RAM, Swap, Load Average, Battery telemetry) natively across Linux, macOS, and BSD flavors.
* Environment Aware: Monitors active SSH connections, terminal plexers (`tmux`/`screen`), nested shell layers (`SHLVL`), and background jobs.
* Modular Shell Framework: Decouples path configurations, customized alias sets, and third-party developer tool initialization hooks into separate, clean runtime scripts.


## Layout Architecture

The prompt layout is built programmatically by the `PromptEngine` via an aggregation of custom modules. Modules are arranged in stackable rows using the `SpacerModule` component:

1. **Top Row (`SpacerModule` 1):** Renders critical system diagnostics—`CMDStatusModule` (exit pipe statuses / execution time), `DateTimeModule`, `LoadAVGModule`, `RAMModule` (including Swap space), and `BatteryModule`. It is prefixed with the `┌─` frame line.
2. **Middle Row (`SpacerModule` 2):** Dedicated to dev-tooling telemetry—displays `GitModule` repository branches/dirty tracking flags, active `CondaModule` virtual environments, and `ESPIDFModule` system targets. It is prefixed with the `│ ` frame line.
3. **Bottom Row (`SpacerModule` 3):** Handles shell session environmental states—`EnvMonitorModule` (SSH connections/Multiplexers/nested shells/jobs count), `UserNameModule` (with root detection overrides), `PathModule` (current directory mapping home onto `~`), and the final terminal `SymbolModule` (`$` or `#`). It is prefixed with the `└─` frame line.

## Building and Installation

### Install from a release

```bash
curl -fsSL https://raw.githubusercontent.com/aruslantsev/beautiful-shell/main/install.sh | sh
```

The installer downloads the matching GitHub release tarball for your OS and architecture, verifies `SHA256SUMS`, unpacks into `~/.beautiful_shell`, and appends the `shellrc` hook to `~/.bashrc` and/or `~/.zshrc` when those files exist. If your current shell is bash or zsh and its RC file is missing, that one file is created with the hook.

```bash
# Specific version
curl -fsSL https://raw.githubusercontent.com/aruslantsev/beautiful-shell/main/install.sh | sh -s -- --version v1.0.0
# or
VERSION=v1.0.0 curl -fsSL https://raw.githubusercontent.com/aruslantsev/beautiful-shell/main/install.sh | sh

# Unpack only; do not edit RC files
curl -fsSL https://raw.githubusercontent.com/aruslantsev/beautiful-shell/main/install.sh | sh -s -- --no-hook
```

### Build from source

You only need a C++17-compliant compiler (GCC or Clang) and CMake (>= 3.15). All external dependencies (`libgit2` and `tomlplusplus`) are automatically downloaded, configured, and statically linked during the build process.

Copy `src/scripts` and `src/shellrc` to `~/.beautiful_shell/`, then compile the binary:

```bash
cmake -S src/beautiful_shell -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Once the compilation succeeds, the self-contained `beautiful_shell` executable will be generated in the `build` directory. Move this binary to `~/.beautiful_shell/bin/`.

Linux release binaries published from CI are fully static musl builds (Alpine). Do not enable `-DBSHELL_STATIC=ON` on a glibc host — NSS/`getpwuid` will fail at runtime.


## Configuration File (`~/.config/beautiful_shell/config`)

The C++ utility automatically searches for its behavior file at `~/.config/beautiful_shell/config`. The file uses the TOML format. If the file is missing or contains syntax errors, the application safely falls back to predefined default settings.

### Configuration Example
```toml
[prompt]
theme = "dark"              # Color theme: "dark" (default) or "light"; "custom" follows dark
use_colors = true           # Toggle ANSI color styling (true/false)
show_root_username = false  # Hide the "root" username string (when uid=0) to save space
time_threshold = 1.0        # Command execution time threshold (in seconds)

[shell]
run_ssh_agent = true        # Toggle starting ssh-agent via the bootloader framework
save_history = false        # Toggle custom history-saving behaviors (incognito mode)
conda_init = true           # Enable tracking of Conda virtual environments

[battery]
show_battery = true         # Enable/disable laptop battery monitoring
battery_warn_percent = 20   # Lower boundary percentage to trigger a red alert color
battery_hide_percent = 80   # Hide the battery block if charged above this threshold while plugged in
```

### Detailed Parameter Reference

#### The `[prompt]` Section
* `theme` *(string)*: Sets the base palette. Modules map specific color profiles based on `"dark"` and `"light"` terminal backgrounds. `"custom"` (and any other value) currently uses the dark palette.
* `use_colors` *(boolean)*: Toggles global ANSI escape sequences. Set to `false` if your terminal environment doesn't support color formatting.
* `show_root_username` *(boolean)*: Defaults to `false`. When running a root shell, the string username can be hidden to save screen space, relying solely on the red `#` prompt symbol instead.
* `time_threshold` *(float)*: If a command takes longer than this value to execute, the utility calculates the runtime and prefixes your prompt with a human-readable timer (e.g., `[1.5s]` or `[2h15m5s]`).

#### The `[shell]` Section
* The parameters `run_ssh_agent`, `save_history`, and `conda_init` are parsed dynamically by the binary's `init` handler. They print environment toggle variables (`_BS_RUN_SSH_AGENT`, etc.) into your initialization stream, establishing a centralized configuration map.

#### The `[battery]` Section
* `show_battery` *(boolean)*: Toggles laptop power monitoring. On desktop workstations without an internal battery interface, this module remains hidden.
* `battery_warn_percent` *(integer)*: If the laptop is discharging and the battery percentage drops below this number, the layout text overrides to vivid red.
* `battery_hide_percent` *(integer)*: If your laptop is plugged into AC power and charged beyond this limit, the battery indicator automatically vanishes to minimize terminal layout noise.


## Shell Bootloader Framework

`beautiful_shell` utilizes a modular shell bootloader framework situated at `~/.beautiful_shell/`. This structure maximizes shell startup execution speeds while parsing user settings cleanly out of framework cores.

### Directory Layout
Place the accompanying script suites within the following path distribution:
```text
~/.beautiful_shell/
├── shellrc                 # Main runtime entrypoint sourced by ~/.bashrc or ~/.zshrc
└── bin/
    ├── beautiful_shell     # Prompt generator 
└── scripts/
    ├── colors              # Global internal ANSI styling escape definitions
    ├── messages            # Log output layout adapters (__bs_print_info, etc.)
    ├── initialize          # Invokes the C++ prompt engine hooks and layouts
    ├── 00_path             # Manages filesystem PATH definitions
    ├── 02_aliases          # Isolates shell-specific or global custom aliases
    ├── 03_environment      # Isolates localized user exports
    ├── 10_ssh-agent        # Manages static long-term SSH daemon sockets
    ├── 20_history          # Dictates incognito transient session controls
    └── 30_devtools         # Configures isolated sandbox paths (Conda, etc.)
```

### Extensible Custom Script Hooks
The framework attempts to discover and source custom user-space dotfiles seamlessly on runtime initialization. You do not need to modify the internal core framework files; simply drop your customization profiles inside your `$HOME` folder:

#### 1. Path Layer Configuration (`scripts/00_path`)
* **Behavior:** Adds `~/.local/bin` to the active `$PATH` environment if the folder exists.
* **User Hook:** Automatically scans for and sources `~/.paths`. Use this file to hold your custom application target exports.

#### 2. Shell Context Aliases (`scripts/02_aliases`)
* **Behavior:** Sources structural command shortcuts.
* **Global User Hook:** Sources `~/.aliases` (Shared shortcuts shared across all POSIX shell installations).
* **Context User Hook:** Sources `~/.bash_aliases` or `~/.zsh_aliases` dynamically depending on the current shell.

#### 3. Personal Environment Definitions (`scripts/03_environment`)
* **Behavior:** Extracts heavy user tokens or runtime exports out of terminal configurations.
* **User Hook:** Sources `~/.environment`. Ideal for variables like `export EDITOR="vim"` or `export GPG_TTY=$(tty)`.


## Advanced Shell Automation Modules

### Automated Static `ssh-agent` Socket (`scripts/10_ssh-agent`)
* **Condition:** Activates if `_BS_RUN_SSH_AGENT=1` and a local `~/.ssh` directory is found.
* **Features:**
  * **Dead Descriptor Guard:** Automatically detects and purges orphaned socket references (`~/.ssh/ssh-agent.sock`) if a previous terminal host window or system session crashed unexpectedly.
  * **Persistent Shared Sock:** Links all split shells, multiplexer windows (`tmux`/`screen`), and active parent contexts to a single, static UNIX socket file. Passphrases only need to be input **once** per OS lifecycle boot.
  * **Config Auditor:** Automatically checks your `~/.ssh/config` architecture and alerts you with a warning if the `AddKeysToAgent yes` parameter is missing.

### Dynamic History Controls (`scripts/20_history`)
* **Condition:** Activates secure incognito protections if `_BS_SAVE_HISTORY=0`.
* **Features:** Disables historical disk footprints completely for fast, private terminal routines:
  * **Bash / Zsh:** Safely unsets the `$HISTFILE` parameter.
  * **Sh / Ash:** Reroutes the `$HISTFILE` destination securely to `/dev/null`.

### Conda Sandbox Isolation (`scripts/30_devtools`)
* **Condition:** Activates if `_BS_CONDA_INIT=1` and a `~/miniconda3` installation directory exists.
* **Features:**
  * Automates Miniconda initialization hooks using real-time environment evaluation.
  * **Visual Layout Overrides Control:** Configures your local `~/.condarc` dynamically to enforce `changeps1: False`. This prevents the default Python virtualenv wrapper from forcefully injecting the bulky raw `(base)` text prefix into your command line, allowing your `beautiful_shell` binary module to elegantly format it instead.


## Shell Integration Deployment

The release installer appends this hook to `~/.bashrc` and/or `~/.zshrc` when those files exist (use `--no-hook` to skip). For a from-source install, append the same block yourself:

### For Bash (`~/.bashrc`)
```bash
if [ -f ~/.beautiful_shell/shellrc ]; then
    . ~/.beautiful_shell/shellrc
fi
```

### For Zsh (`~/.zshrc`)
```bash
if [ -f ~/.beautiful_shell/shellrc ]; then
    . ~/.beautiful_shell/shellrc
fi
```
