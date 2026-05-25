#include <stdio.h>
// #include <stdlib.h>
#include <string.h>
#include <time.h>
#include "argparser.h"


#include "colors.h"
#include "beautiful_prompt.h"

/*
# ┌─┬─┐
# │ │ │
# ├─┼─┤
# │ │ │
# └─┴─┘
*/


int main(int argc, char *argv[]) {
    struct prompt_data data;
    data.execution_time = -1;
    data.return_code = 0;
    data.pipe_status = 0;
    data.num_jobs = 0;
    data.shlvl = 1;
    if (parser(&data, argc, argv) != 0) {
        fprintf(stderr, "%s\n", "Error parsing arguments");
    }
    if (get_user_info(&data) != 0) {
        fprintf(stderr, "%s\n", "Could not get user data");
    }
    if (get_hostname(&data) != 0) {
        fprintf(stderr, "%s\n", "Could not get host data");
    }
    if (get_cwd(&data) != 0) {
        fprintf(stderr, "%s\n", "Could not get path data");
    }
    if (get_localtime(&data) != 0) {
        fprintf(stderr, "%s\n", "Could not get time");
    }
    if (get_battery_info(&data) != 0) {
        fprintf(stderr, "%s\n", "Could not get battery info");
    }


    const char *prompt_sym = data.is_root ? "#" : "$";
    const char *ok_color = data.is_root ? GREY : GREEN;
    const char *err_color = RED;

    printf("┌─ %s[%d", data.return_code == 0 && data.pipe_status == 0 ? ok_color : err_color, data.return_code);
    if (data.pipe_status != 0) {
        printf("|P%d", data.pipe_status);
    }
    if (data.execution_time >= 0) {
        printf("|%ds", data.execution_time);  /* FIXME: convert to minutes, days, ... */
    }
    printf("]%s ", RESET);
    printf("%s%s%s ", GREY, data.localtime, RESET);
    printf("\n");

    printf("│ ");  /* FIXME: create buffer for each string. snprintf inside. Print string if not empty*/
    if (data.bat_status == DISCHARGING) {
        printf("%s[BAT:%d]%s ", data.bat_percent >= 20 ? GREY : YELLOW, data.bat_percent, RESET);
    }
    char *is_ssh = getenv("SSH_CONNECTION");
    if (is_ssh != NULL) {
        printf("%s[%s]%s ", YELLOW, "SSH", RESET);
    }
    if (data.num_jobs != 0) {
        printf("%s[J%d]%s ", YELLOW, data.num_jobs, RESET);
    }
    if (data.shlvl != 1) {
        printf("%s[LVL%d]%s ", YELLOW, data.shlvl, RESET);
    }
    printf("\n");

    printf("└─ ");
    if (data.is_root) {
        if (strlen(data.hostname) > 0) {
            printf("%s%s%s", RED, data.hostname, RESET);
        }
        printf(" ");
    } else {
        if (strlen(data.username) > 0) {
            printf("%s%s%s", GREEN, data.username, RESET);
        }
        if (strlen(data.username) > 0 && strlen(data.hostname) > 0) {
            printf("%s%s%s", GREEN, "@", RESET);
        }
        if (strlen(data.hostname) > 0) {
            printf("%s%s%s", GREEN, data.hostname, RESET);
        }
        if (strlen(data.username) > 0 || strlen(data.hostname) > 0) {
            printf(" ");
        }
    }
    
    if (strlen(data.cwd) > 0) {
        printf("%s%s%s ", BLUE, data.cwd, RESET);
    }
    printf("%s%s%s ", BLUE, prompt_sym, RESET);
    return 0;
}


/* TMUX */
/* 

_PR_BIN="/home/andrei/git/beautiful-shell/beautiful_shell/scripts/sh/beautiful_prompt"

if [ -n "$ZSH_VERSION" ]; then
    preexec() { 
        _T_START=$(date +%s) 
    }
    precmd() {
        local last_status=$? # Сохраняем сразу
        local _T_END=$(date +%s)
        local _T_EXEC=0
        
        if [ -n "$_T_START" ]; then
            _T_EXEC=$(( _T_END - _T_START ))
        fi

        local p_arg=()
        if (( ${#pipestatus} > 1 )); then
             p_arg=("-p" "${(j: :)pipestatus}")
        fi

        PROMPT="$($_PR_BIN -c "$last_status" -t "$_T_EXEC" -s "$SHLVL" "${p_arg[@]}")"
        unset _T_START
    }
elif [ -n "$BASH_VERSION" ]; then
    _b_preexec() {
        [ -n "$COMP_LINE" ] && return
        [ -z "$BASH_COMMAND" ] && return
        [ "$BASH_COMMAND" = "$PROMPT_COMMAND" ] && return
        
        if [ -z "$_T_START" ]; then
            _T_START=$(date +%s)
        fi
    }
    _b_precmd() {
        local last_status=$?
        
        local _T_END=$(date +%s)
        local _T_EXEC=0

        if [ -n "$_T_START" ]; then
            _T_EXEC=$(( _T_END - _T_START ))
        fi

        jobs &>/dev/null
        local job NUM_JOBS=0 IFS=$' \t\n'
        for job in $(jobs -p); do [[ $job ]] && ((NUM_JOBS++)); done

        PS1="$($_PR_BIN -c "$last_status" -t "$_T_EXEC" -j "$NUM_JOBS" -s "$SHLVL")"
        unset _T_START
    }
    trap '_b_preexec' DEBUG
    PROMPT_COMMAND="_b_precmd; $PROMPT_COMMAND"
else
    _a_precmd() {
        $(_PR_BIN -c $?)
    }
    PS1='$(_a_precmd)'
fi

*/
