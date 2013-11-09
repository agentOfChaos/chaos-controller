#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/user.h>
#include <errno.h>
#include <signal.h>
#include <linux/limits.h>
#include "constants.h"
#include "prototypes.h"

long offset;
const int long_size = sizeof(long);
const int int_size = sizeof(int);

unsigned long long int libc_posiz(pid_t pid)
{
    char shellcommand[120];
    char numero[60];
    char directory[PATH_MAX];
    int reto;
    FILE * leggi;

    getwd(directory);
    sprintf(shellcommand,"%s/mrmaps %d",directory,pid);

    leggi=popen(shellcommand,"re");
    if (leggi==NULL)
        {
        printf("Error locating libc. (0)\n");
        exit(-1);
        }
    fread(numero,sizeof(char),60,leggi);
    reto=pclose(leggi);

    if (reto==666)
        {
        printf("Error locating libc. (1)\n");
        exit(-2);
        }
    return strtoll(numero, (char **)NULL,16);
}
int aggancia(pid_t pid)
{
    int ret;
    ret=ptrace(PTRACE_ATTACH,pid,NULL,NULL);
    if (ret!=-1)
        {
        wait(NULL);
        }
    return ret;
}
unsigned long long int self_discover() // get a pointer to the rand() function in libc
{
    void *altro;
    altro=dlsym(RTLD_NEXT,"rand");
    return (unsigned long long int)altro;
}
int main(int argc, char *argv[])
{
    pid_t pid;
    int mode;
    char ans;
    int forkmode=0;
    int offsetmode=0;
    unsigned long long int libc_addr,bersaglio,my_libc,recalc;
    int valore_eax;
    parse_cmdline(argc,argv,&mode,&forkmode,&pid,&offsetmode,&valore_eax);

    my_libc=libc_posiz(getpid()); // localizza libc nel processo corrente (per la offsetmode auto)

    if (offsetmode==0)offset=offset_arch; // <<<<< set this on compile time
    else
        {
        printf("%lx +4 - %lx = ",self_discover(),my_libc);
        recalc=(self_discover()+4)-my_libc;
        offset=(long)recalc;
        }
    printf("Function offset: %lx\n",offset);

    if (forkmode) // se in modalità fork, spawna il processo
        {
        if (spawn(argv[2],&pid)==-1)
            {
            printf("Cannot spawn the process.\n");
            exit(-3);
            }
        printf("Spawned process with pid=%d\n",pid);
        }

    if (controlla_pid(pid)==0) // controlla se esiste in /proc (mi serve /proc/$pid/maps !)
        {
        printf("unexistent process.\n");
        exit(-4);
        }
    libc_addr=libc_posiz(pid); // localizza libc nel processo target
    printf("Process' base (libc): %lx\n",libc_addr);
    bersaglio=libc_addr+offset;
    printf("Injection target: %lx\n",bersaglio);

    if (aggancia(pid)==-1)
        {
        printf("Cannot attach the process. Try running in as sudo.\n");
        exit(-5);
        }
    else printf("Process successfully attached.\n");
    if (!safecheck(pid,bersaglio) && (mode!=2))
        {
        printf("Unexpected result! Recognitions report a target memory word that does not match the expected one.\n");
        printf("Do you wish to continue, my master? (y/n) ");
        scanf("%c",&ans);
        getchar();
        if (ans!='y' && ans!='Y')exit(-3);
        }
    do
        {
        ans='n';
        switch (mode)
            {
            case 0: nopmode(pid,bersaglio); break;
            case 1: customnum(pid,bersaglio,valore_eax); break;
            default: restore(pid,bersaglio);
            }
        printf("Done injection!\n");
        if (forkmode)
            {
            ptrace(PTRACE_DETACH,pid,NULL,NULL);
            printf("inject again? (y/n) ");
            scanf("%c",&ans); getchar();
            if (ans=='y' || ans=='Y')
                {
                printf("mode? 0=nop 1=num 2=fix : ");
                scanf("%d",&mode);
                if (mode==1)
                    {
                    printf("Return value? ");
                    scanf("%d",&valore_eax); getchar();
                    }
                }
            if (aggancia(pid)==-1)
                {
                printf("Cannot attach the process. (Did it terminate?).\n");
                exit(-5);
                }
            }
        } while (ans=='y' || ans=='Y');

    if (forkmode)
        {
        printf("Kill the child process on exit? (y/n) ");
        scanf("%c",&ans);
        getchar();
        if (ans=='y' || ans=='Y')
            {
            ptrace(PTRACE_DETACH,pid,NULL,NULL);
            kill(pid,SIGKILL); // extreme pregiudice!
            printf("Done!\n");
            }
        else
            {
            printf("Init will do it for you.\n");
            ptrace(PTRACE_DETACH,pid,NULL,NULL);
            }
        }
    else ptrace(PTRACE_DETACH,pid,NULL,NULL);
    printf("Process %d detached. Exiting.\n",pid);
    return 0;
}
