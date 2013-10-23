
/* Sample TCP server */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char**argv)
{
    int listenfd;
    int connfd;
    int n;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    pid_t     childpid;
    char mesg[1000];


    printf("Server start on port: %d\n",  atoi(argv[1]));


    listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1){
        printf("Error creating socket\n");
        return 0;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(atoi(argv[1]));

    bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    if(listen(listenfd,1024) == -1){
        printf("Error listening");
    }
    for(;;){
        clilen=sizeof(cliaddr);
        connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
        if ((childpid = fork()) == 0){
            close (listenfd);
            for(;;){
                n = recvfrom(connfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&clilen);
                sendto(connfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                if(mesg != NULL){
                    if(strstr(mesg,"quit")){
                        close(connfd);
                        printf("Disconnect process %d",connfd);
                        exit(childpid);
                    }
                    else{
                        printf("-------------------------------------------------------\n");
                        mesg[n] = 0;
                        printf("Received the following:\n");
                        printf("%s",mesg);
                        printf("-------------------------------------------------------\n");
                    }
                }
            }
        }
    }
}
