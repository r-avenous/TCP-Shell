// Assignment 2 Q 2
// 20CS10012
// Atulya Sharma
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>

#define MAXLINE 50
#define PORT 8009
#define MAXPARALLELCONNECTIONS 5

typedef struct _dynamic_string
{
    char *str;
    int size;
}dynstr;
// dynamic string code
void append(dynstr* ds, char* next)
{
    if(ds->str == NULL)
    {
        ds->str = (char *) malloc(sizeof(char) * (strlen(next) + 1));
        strcpy(ds->str, next);
        ds->size = strlen(next) + 1;
    }
    else
    {
        ds->str = (char *) realloc(ds->str, sizeof(char) * (ds->size + strlen(next) + 1));
        strcat(ds->str, next);
        ds->size += strlen(next) + 1;
    }
}
// trie indexer and code
int indexer(char c)
{
    static char special[] = {'_', '-', '.', '!', '~', '*', '\'', '(', ')', ';', ':', '@', '&', '=', '+', '$', ',', '/', '?', '#', '[', ']', '%', ' '};
    if(c >= 'a' && c <= 'z')
        return c - 'a';
    else if(c >= 'A' && c <= 'Z')
        return c - 'A' + 26;
    else if(c >= '0' && c <= '9')
        return c - '0' + 52;
    else
    {
        for(int i = 0; i < 24; i++)
            if(c == special[i])
                return i + 62;
    }
}
typedef struct _trieNode
{
    struct _trieNode *next[96];
    int isEnd;
}trieNode;

void insert(trieNode *root, char *word)
{
    trieNode *temp = root;
    for(int i = 0; i < strlen(word) - 1; i++)
    {
        int index = indexer(word[i]);
        if(temp->next[index] == NULL)
        {
            temp->next[index] = (trieNode*)malloc(sizeof(trieNode));
            temp = temp->next[index];
            temp->isEnd = 0;
            for(int j = 0; j < 96; j++)
                temp->next[j] = NULL;
        }
        else
            temp = temp->next[index];
    }
    temp->isEnd = 1;
}

int search(trieNode *root, char *word)
{
    trieNode *temp = root;
    for(int i = 0; i < strlen(word); i++)
    {
        int index = indexer(word[i]);
        if(temp->next[index] == NULL)
            return 0;
        else
            temp = temp->next[index];
    }
    return temp->isEnd;
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0), newsockfd, clilen, bytes;
    struct sockaddr_in serv_addr, cli_addr;
    char buf[MAXLINE];
    // file handled to insert users into the trie
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(0);
    }
    trieNode *root = (trieNode*)malloc(sizeof(trieNode));
    root->isEnd = 0;
    for(int i = 0; i < 96; i++) root->next[i] = NULL;

    while (fgets(buf, sizeof(buf), fp) != NULL) 
    {
        insert(root, buf);
    }
    fclose(fp);

    if(sockfd < 0)
    {
        perror("Cannot create socket ");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error binding socket ");
        exit(0);
    }

    listen(sockfd, MAXPARALLELCONNECTIONS);

    while(1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        printf("New Connection Arrived\n");
        if(newsockfd < 0)
        {
            perror("Error accepting ");
            exit(0);
        }
        if(fork() == 0)
        {
            close(sockfd);
            strcpy(buf, "LOGIN: ");
            send(newsockfd, buf, MAXLINE, 0);
            dynstr uname;
            uname.str = NULL;
            uname.size = 0;
            int unameReceived = 0;
            while(!unameReceived)
            {
                recv(newsockfd, buf, MAXLINE, 0);
                for(int i=0; i<MAXLINE; i++) if(buf[i] == 0) unameReceived = 1;
                append(&uname, buf);
            }
            printf("Username: %s\n", uname.str);
            if(search(root, uname.str))
            {
                strcpy(buf, "FOUND");
                send(newsockfd, buf, MAXLINE, 0);
                while(1)
                {
                    dynstr cmdf;
                    cmdf.str = NULL;
                    cmdf.size = 0;
                    int received = 0;
                    while(!received)
                    {
                        recv(newsockfd, buf, MAXLINE, 0);
                        for(int i=0; i<MAXLINE; i++) if(buf[i] == 0) received = 1;
                        append(&cmdf, buf);
                    }
                    if(strcmp(cmdf.str, "exit") == 0)
                    {
                        printf("User Exited\n");
                        break;
                    }
                    printf("Command: %s\n", cmdf.str);
                    char *s = NULL, *cmd = cmdf.str, res[1000];
                    while(*cmd == ' ') cmd++;
                    if(!strcmp(cmd, "pwd")) 
                    {
                        sprintf(res, "%s", getcwd(s, 0));
                    }
                    else
                    {
                        if(cmd[0] == 'c' && cmd[1] == 'd')
                        {
                            char *dir = cmd + 3;
                            while(*dir == ' ') dir++;
                            int f = chdir(dir);
                            if(f == -1) sprintf(res, "####");
                            else res[0] = 0;
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
                    int index = 0, len = strlen(res);
                    while(index <= len)
                    {
                        if(index == len) 
                        {
                            buf[0] = 0;
                            index++;
                        }
                        else
                        {
                            int i;
                            for(i=0; i<MAXLINE && index<len; i++, index++)
                                buf[i] = res[index];
                            if(i < MAXLINE) 
                            {
                                buf[i] = 0;
                                index++;
                            }
                        }
                        send(newsockfd, buf, MAXLINE, 0);
                    }
                }
            }
            else
            {
                strcpy(buf, "NOT-FOUND");
                printf("User Not Found\n");
                send(newsockfd, buf, MAXLINE, 0);
            }
            close(newsockfd);
            exit(0);
        }
        else 
        {
            close(newsockfd);
        }
    }
    return 0;
}