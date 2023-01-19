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

#define MAXLINE 50
#define PORT 8009
#define HOME "127.0.0.1"

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0), bytes;
    struct sockaddr_in servaddr;
    if(sockfd < 0)
    {
        perror("Error Creating Socket ");
        exit(0);
    }
    servaddr.sin_family = AF_INET;
    inet_aton(HOME, &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);

    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
    {
        perror("Error Connecting to Server ");
        exit(0);
    }
    char buf[MAXLINE], *uname = NULL, cmd[400];
    bytes = recv(sockfd, buf, MAXLINE, 0);
    buf[bytes] = 0;
    printf("%s\n", buf);
    bytes = 0;
    scanf("%[^\n]", buf);
    send(sockfd, buf, MAXLINE, 0);
    recv(sockfd, buf, MAXLINE, 0);
    if(!strcmp(buf, "FOUND"))
    {
        while(1)
        {
            printf("Enter Command: ");
            getchar();
            scanf("%[^\n]", cmd);
            int index = 0, len = strlen(cmd);
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
                        buf[i] = cmd[index];
                    if(i < MAXLINE) 
                    {
                        buf[i] = 0;
                        index++;
                    }
                }
                send(sockfd, buf, MAXLINE, 0);
            }
            if(!strcmp(cmd, "exit")) break;
            int received = 0;
            while(!received)
            {
                char res[MAXLINE+1];
                res[MAXLINE] = 0;
                recv(sockfd, res, MAXLINE, 0);
                for(int i=0; i<MAXLINE; i++) if(res[i] == 0) received = 1;
                printf("%s", res);
            }
            printf("\n");
        }
    }
    else printf("Invalid username\n");
    close(sockfd);
    return 0;
}