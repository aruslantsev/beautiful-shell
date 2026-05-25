#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define ARGSIZE     64


int main(int argc, char *argv[]) 
{
    char argname[ARGSIZE];
    char argval[ARGSIZE];
    int argnum = 1;  /* skip program name */
    while (argnum < argc) {
        bool is_short;
        printf("ARG %s\n", argv[argnum]);
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
            strncpy(argname, argv[argnum], strchr(argv[argnum], '=') - argv[argnum]);
            argname[strchr(argv[argnum], '=') - argv[argnum]] = '\0';
            strcpy(argval, strchr(argv[argnum], '=') + 1);
        } else {
            strcpy(argname, argv[argnum]);
            argnum++;
            if (argnum >= argc) {
                fprintf(stderr, "Missing value for key: %s\n", argv[argnum - 1]);
                exit(EXIT_FAILURE);
            }
            strcpy(argval, argv[argnum]);
        }
        printf("Arg name: '%s', arg val: '%s'\n", argname, argval);
        if (is_short && strlen(argname) != 2) {
            fprintf(stderr, "Invalid short key: %s\n", argname);
        }
        
        /* parse here */
        argnum++;
    }
    return 0;
}