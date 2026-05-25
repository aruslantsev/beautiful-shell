#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "beautiful_prompt.h"

#define ARGSIZE     64


int parser(struct prompt_data *data, int argc, char *argv[]) 
{
    char argname[ARGSIZE];
    char argval[ARGSIZE];
    int argnum = 1;  /* skip program name */
    for (int i = 0; i < argc; printf("%s\n", argv[i++]));
    while (argnum < argc) {
        bool is_short;

        if (strlen(argv[argnum]) < 2) {
            fprintf(stderr, "Invalid argument: %s\n", argv[argnum]);
            exit(EXIT_FAILURE);
        }
        if (argv[argnum][0] != '-') {
            fprintf(stderr, "Not a key: %s\n", argv[argnum]);
            exit(EXIT_FAILURE);
        }
        if (argv[argnum][1] != '-') {
            is_short = true;
        } else {
            is_short = false;
        }
        if (strchr(argv[argnum], '=') != NULL) {
            int argname_length = strchr(argv[argnum], '=') - argv[argnum];
            if (argname_length > ARGSIZE) {
                argname_length = ARGSIZE;
            }
            strncpy(argname, argv[argnum], argname_length);
            argname[argname_length] = '\0';
            argname[ARGSIZE - 1] = '\0';
            strncpy(argval, strchr(argv[argnum], '=') + 1, ARGSIZE);
            argval[ARGSIZE - 1] = '\0';
        } else {
            strncpy(argname, argv[argnum], ARGSIZE);
            argname[ARGSIZE - 1] = '\0';
            argnum++;
            if (argnum >= argc) {
                fprintf(stderr, "Missing value for key: %s\n", argv[argnum - 1]);
                exit(EXIT_FAILURE);
            }
            strncpy(argval, argv[argnum], ARGSIZE);
            argval[ARGSIZE - 1] = '\0';
        }
        if (is_short && strlen(argname) != 2) {
            fprintf(stderr, "Invalid short key: %s\n", argname);
        }
        
        if (strcmp(argname, "-t") == 0 || strcmp(argname, "--execution-time") == 0) {
            data->execution_time = atoll(argval);
        }
        if (strcmp(argname, "-c") == 0 || strcmp(argname, "--return-code") == 0) {
            data->return_code = atoi(argval);
        }
        if (strcmp(argname, "-j") == 0 || strcmp(argname, "--num-jobs") == 0) {
            data->num_jobs = atoi(argval);
        }
        if (strcmp(argname, "-s") == 0 || strcmp(argname, "--shlvl") == 0) {
            data->shlvl = atoi(argval);
        }
        if (strcmp(argname, "-p") == 0 || strcmp(argname, "--pipe-status") == 0) {
            char buffer[ARGSIZE];
            strncpy(buffer, argval, ARGSIZE);
            buffer[ARGSIZE - 1] = '\0';
            char *token = strtok(buffer, " ");
            while (token != NULL) {
                int8_t code = atoi(token);
                if (code != 0) {
                    data->pipe_status = code;
                    break;
                }
                token = strtok(NULL, " ");
            }
        }
        argnum++;
    }
    return 0;
}