#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "prototypes.h"

void printhelpNquit(char *name)
{
    printf("Chaos controller  v1.3  by agentOfChaos  informaniac99<AT>gmail<DOT>com\n");
    printf("Small linux utility that lets you hack the libc random number generator, bypassing the C random() function and altering the rand()'s return value.\n");
    printf("In layman terms: if random number is god, you can be god too!\n\n");
    printf("Usage: %s [arguments]\n",name);
    printf("Mandatory arguments: offset_mode target inject_mode\n");
    printf("--help -h -?\tprint this help screen\n.");
    printf("[offset_mode]\t\"auto\" OR \"preset\"\n");
    printf("How the program will know the memory offset (from libc beginning) where to find rand() entry point.\n");
    printf("\tpreset will use the value defined at compile time.\n");
    printf("\tauto will guess it based on it's location in the current program.\n");
    printf("[target]\tpid OR absolute_path\n");
    printf("The target process to manipulate\n");
    printf("\tpid of a running process. keep in mind however that you might require root privilege for that\n");
    printf("\tabsolute_path of a child process to spawn\n");
    printf("[inject_mode]\tnop OR num OR fix\n");
    printf("How the program will alterate target's memory\n");
    printf("\tnop will replace the call-to-random with NOPs (assembly opcode that does nothing)\n");
    printf("\tfix will restore the default value (a call to the random subroutine)\n");
    printf("\tnum [my_number] will replace the call-to-random with MOV [my_number] to eax, basically making rand return your own number of choice.\n");
    printf("\nExample usage (sudo may be required):\n\t%s auto $(pidof killbots) num 4\n",name);
    printf("Then, to restore the program's unaltered memory: \n\t%s auto $(pidof killbots) fix\n",name);
    exit(0);
}
int opt_help(char *opt)
{
    if (strcmp(opt,"-h")==0) return 1;
    if (strcmp(opt,"--help")==0) return 1;
    if (strcmp(opt,"-?")==0) return 1;
    return 0;
}
void parse_cmdline(int *argc, char *argv[], int *mode, int *forkmode, pid_t *pidtarg, int *offsetmode, int *valeax)
{
    int cont;

    if (argc<=1) printhelpNquit(argv[0]);

    for (cont=1; cont<argc; cont++)
        {
        if (opt_help(argv[cont]))
            {
            printhelpNquit(argv[0]);
            }
        }

    if (strcmp(argv[1],"preset")==0)*offsetmode=0; // argomento 1: modalità offset
    else if (strcmp(argv[1],"auto")==0)*offsetmode=1;
    else printhelpNquit(argv[0]);

    if (isname(argv[2]))(*forkmode)=1; // argomento 2: bersaglio
    else if (argv[2]==NULL) printhelpNquit(argv[0]);
    else *pidtarg=atoi(argv[2]);

    if (argv[3]==NULL) printhelpNquit(argv[0]); // argomento 3: modalità iniezione
    else if (strcmp(argv[3],"nop")==0) *mode=0;
    else if (strcmp(argv[3],"num")==0) *mode=1;
    else if (strcmp(argv[3],"fix")==0) *mode=2;
    else printhelpNquit(argv[0]);

    if ((*mode)==1) // argomento 4: valore custom (opzionale)
        {
        if (argv[4]==NULL) printhelpNquit(argv[0]);
        else *valeax=atoi(argv[4]);
        }
}
