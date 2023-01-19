#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

int main()
{
    while(1)
    {

    char *s = NULL, *cmd, res[400];
    cmd = (char *)malloc(100);
    scanf("%[^\n]", cmd);
    while(*cmd == ' ') cmd++;
    if(!strcmp(cmd, "pwd")) 
    {
        sprintf(res, "%s", getcwd(s, 0));
    }
    else
    {
        if(cmd[0] == 'c' && cmd[1] == 'd')
        {
            char *dir = cmd + 2;
            while(*dir == ' ') dir++;
            int f = chdir(dir);
            if(f == -1) sprintf(res, "####");
            // sprintf(res, "%s", getcwd(s, 0));
        }
        else if(cmd[0] == 'd' && cmd[1] == 'i' && cmd[2] == 'r')
        {
            char *dir = cmd + 3;
            while(*dir == ' ') dir++;
            DIR *d;
            struct dirent *de;
            if(*dir == '\0') d = opendir("./");
            else d = opendir(dir);
            if(d == NULL) sprintf(res, "####");
            else
            {
                int i = 0;
                while((de = readdir(d)) != NULL)
                {
                    if(i) sprintf(res, "%s %s", res, de->d_name);
                    else sprintf(res, "%s", de->d_name);
                    i++;
                }
            }
        }
        else sprintf(res, "$$$$");
    }
    printf("%s\n", res);
    getchar();

    }
    return 0;
}