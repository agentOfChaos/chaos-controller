#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "prototypes.h"

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
