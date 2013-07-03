#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/user.h>
#include <errno.h>
#include <signal.h>
#include <linux/limits.h>

#define offset 0x3C614
#define shell_path "/bin/bash"

const int long_size = sizeof(long);
const int int_size = sizeof(int);
int forkmode=0;

int controlla_pid(pid_t pid)
{
    char dest[60];
    FILE *apri;
    sprintf(dest,"/proc/%d/maps",pid);
    apri=fopen(dest,"r");
    if (apri!=NULL)
        {
        fclose(apri);
        return 1;
        }
    else return 0;
}
unsigned long long int fork_n_read(pid_t pid)
{
    int readpipe [2] = {-1,-1};	/* child -> parent */
    char shellcommand[120];
    char numero[60];
    char directory[PATH_MAX];
    int reto;

    #define	PARENT_READ	readpipe[0]
    #define	CHILD_WRITE	readpipe[1]

    pid_t childpid;
    getwd(directory);

    if (pipe(readpipe) < 0)
        {
        printf("Pipe error.\n");
        exit(-4);
        }
    if ( (childpid = fork()) < 0)
        {
        printf("Fork error.\n");
        exit(-5);
        }
    else if (childpid==0)
        {
        close(PARENT_READ);
        dup2(CHILD_WRITE, 1);  close(CHILD_WRITE);
        close(0);
        sprintf(shellcommand,"%s/mrmaps %d",directory,pid);
        execl(shell_path,shell_path,"-c",shellcommand,NULL);
        printf("Execl error!\n");
        exit(666);
        }
    else
        {
        close(CHILD_WRITE);
        wait(&reto); reto=reto/256;
        read(PARENT_READ,numero,60);
        if (reto==666)
            {
            printf("Errore nella child: %s",numero);
            close(PARENT_READ);
            exit(-7);
            }
        else
            {
            close(PARENT_READ);
            return strtoll(numero, (char **)NULL,16);
            }
        }
}
int isname(char *text) // if string contains at least 1 alphabetic character, assume is a filename
{
    int l=strlen(text);
    int z;
    for (z=0; z<l; z++)
        {
        if (isalpha(text[z]))return 1;
        }
    return 0;
}
unsigned long long int prendi_addr(pid_t pid) // deprecated and ugly
{
    char esegui[120];
    char numero[60];
    char directory[PATH_MAX];
    FILE *apri;
    getwd(directory);
    sprintf(esegui,"%s/mrmaps %d",directory,pid);
    //printf("<%s>\n",esegui);
    system(esegui);
    apri=fopen("/tmp/mrmaps_tubo","r");
    if (apri!=NULL)
        {
        fgets(numero,60,apri);
        fclose(apri);
        remove("/tmp/mrmaps_tubo");
        return strtoll(numero, (char **)NULL,16);
        }
    else return -1;
}
void getdata(pid_t child, long addr,
             char *str, int len) // those two 'sister' functions to work on the memory aren't mine, but borrowed from an article on linuxjournal. Open source is like this. Yap.
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA, child,
                          addr + i * 4, NULL);
        memcpy(laddr, data.chars, long_size);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, child,
                          addr + i * 4, NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}
void putdata(pid_t child, long addr,
             char *str, int len)
{   char *laddr;
    int i, j;
    union u {
            long val;
            char chars[long_size];
    }data;
    i = 0;
    j = len / long_size;
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, long_size);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
        ++i;
        laddr += long_size;
    }
    j = len % long_size;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
    }
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
int spawn(char *nomefile,pid_t *pid)
{
    //char path[PATH_MAX];
    pid_t child=fork();
    if (child<0)
        {
        printf("Fork error.\n");
        return -1;
        }
    if (child==0)
        {
        execl(nomefile,nomefile,NULL);
        printf("Exec error.\n");
        exit(-1);
        }
    else
        {
        *pid=child;
        return 1;
        }
}
void nopmode(pid_t pid,unsigned long long int bersaglio)
{
    char iniettando[]={0x90,0x90,0x90,0x90,0x90};
    int i;
    char salvato[9];
    getdata(pid, bersaglio, salvato, 8);
    printf("prima: ");
    for (i=0; i<8; i++)printf("%x ",salvato[i]);
    printf("\ndopo: ");
    for (i=0; i<5; i++){salvato[i]=iniettando[i]; printf("%x ",iniettando[i]);}
    printf("\n");
    putdata(pid,bersaglio,salvato,8);
}
void customnum(pid_t pid,unsigned long long int bersaglio, int valore_eax)
{
    printf(" ---> %lx\n",bersaglio);
    union u {
            int val;
            char chars[int_size];
    }data;
    char salvato[9];
    char iniettando[6];
    int i;
    char opcode=0xb8;
    getdata(pid, bersaglio, salvato, 8);
    printf("prima: ");
    for (i=0; i<8; i++)printf("%x ",salvato[i]);
    data.val=valore_eax;
    iniettando[0]=opcode;
    for (i=0; i<int_size; i++)iniettando[i+1]=data.chars[i];
    iniettando[i+1]=0;
    printf("\ndopo: ");
    for (i=0; i<int_size+1; i++){salvato[i]=iniettando[i]; printf("%x ",iniettando[i]);}
    printf("\n");
    putdata(pid,bersaglio,salvato,8);
}
void restore(pid_t pid,unsigned long long int bersaglio)
{
    char iniettando[]={0xe8,0xb7,0xfa,0xff,0xff};
    int i;
    char salvato[9];
    getdata(pid, bersaglio, salvato, 8);
    printf("prima: ");
    for (i=0; i<8; i++)printf("%x ",salvato[i]);
    printf("\ndopo: ");
    for (i=0; i<5; i++){salvato[i]=iniettando[i]; printf("%x ",iniettando[i]);}
    printf("\n");
    putdata(pid,bersaglio,salvato,8);
}
int safecheck(pid_t pid,unsigned long long int bersaglio)
{
    char normale[]={0xe8,0xb7,0xfa,0xff,0xff,'\0'};
    char leggi[6];
    getdata(pid, bersaglio, leggi, 5);
    //printf("\t%x%x%x\n\t%x%x%x\n",normale[0],normale[1],normale[2],leggi[0],leggi[1],leggi[2]);
    if (strcmp(normale,leggi)!=0) return 0;
    return 1;
}
int main(int argc, char *argv[])
{
    pid_t pid;
    int mode;
    char ans;
    unsigned long long int libc_addr,bersaglio;
    int valore_eax;
    if (argc>=2)
        {
        if (strcmp(argv[1],"-h")!=0 && strcmp(argv[1],"--help")!=0)
            {
            if (isname(argv[1]))
                {
                printf("Fork mode is on!\n");
                forkmode=1;
                }
            pid=atoi(argv[1]);
            }
        else
            {
            printf("First argument: target pid. If the argument is alphanumeric, the program will interpret it as full executable path of a child process to spawn (then attach)\n");
            printf("Second argument: injection mode [0-2]\n");
            printf("Third argument: (if mode is set to 1) return value to inject\n");
            }
        }
    else
        {
        printf("pid of target process: ");
        scanf("%d",&pid);
        getchar();
        }
    if (forkmode){
    if (spawn(argv[1],&pid)==-1)
            {
            printf("Cannot spawn the process.\n");
            exit(-3);
            }}
    if (controlla_pid(pid)==0)
        {
        printf("unexistent process.\n");
        exit(-1);
        }
    libc_addr=fork_n_read(pid);
    if (libc_addr==-1)
        {
        printf("Unexpected error.\n");
        exit(-2);
        }
    bersaglio=libc_addr+offset;
    printf("Injection targete: %lx\n",bersaglio);
    if (argc>=3)
        {
        mode=atoi(argv[2]);
        }
    else
        {
        printf("Action course?\n\n\t0\tNOP\n\t1\tCustom return value\n\t2\tFix code\n\t\t");
        scanf("%d",&mode);
        getchar();
        }
    if (argc>=4 && mode==1)
        {
        valore_eax=atoi(argv[3]);
        }
    else if (mode==1)
        {
        printf("Numeric value to inject? ");
        scanf("%d",&valore_eax);
        getchar();
        }
        if (aggancia(pid)==-1)
            {
            printf("Cannot attach the process. Try running in as sudo.\n");
            exit(-3);
            }
    if (!safecheck(pid,bersaglio) && mode!=2)
        {
        printf("Unexpected result! Recognitions report a target memory word that does not match the expected one.\n");
        printf("Have you come all this way for safety? (y/n) ");
        scanf("%c",&ans);
        getchar();
        if (ans!='n')exit(-3);
        }
    switch (mode)
        {
        case 0: nopmode(pid,bersaglio); break;
        case 1: customnum(pid,bersaglio,valore_eax); break;
        default: restore(pid,bersaglio);
        }
    printf("Done!\n");
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
    return 0;
}
