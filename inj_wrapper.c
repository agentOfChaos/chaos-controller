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

void nopmode(pid_t pid,unsigned long long int bersaglio)
{
    char iniettando[]={0x90,0x90,0x90,0x90,0x90};
    int i;
    char salvato[9];
    getdata(pid, bersaglio, salvato, 8);
    printf("prima: ");
    for (i=0; i<8; i++)printf("%x ",salvato[i]);
    printf("\n(nopmode) dopo: ");
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
    for (i=0; i<8; i++)printf("%x ",(int)salvato[i]);
    data.val=valore_eax;
    iniettando[0]=opcode;
    for (i=0; i<int_size; i++)iniettando[i+1]=data.chars[i];
    iniettando[i+1]=0;
    printf("\n(custom mode) dopo: ");
    for (i=0; i<int_size+1; i++){salvato[i]=iniettando[i]; printf("%x ",(int)iniettando[i]);}
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
    printf("\n (restore mode)dopo: ");
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
