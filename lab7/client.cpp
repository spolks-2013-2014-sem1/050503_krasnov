#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<iostream>
#include <arpa/inet.h>
#include <fstream>
#include<cstdlib>


#define	BytesPerBlock 100

using namespace std;

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
char* itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */

    if (sign < 0){
        s[i++] = '-';
    }

    s[i] = '\0';
    reverse(s);
    return s;

}

class FileRW
{
    public:

        char fileName[20];

        int fileSize;
        FILE *f;
        char * Header;
        int HeaderSize;
        int blocksNumber ;
        int blockSendSize;
        char fileInf[BytesPerBlock+4];

    public:

    FileRW(char *filename)
    {
        strcpy(this->fileName ,filename);
        blocksNumber = 0;

    }

    //функция открытия файла и определения его размера
    int openFile()
    {
        f = fopen(fileName, "r");

        if (f == NULL)
        {
            perror ("Error opening file");
            return -1;
        }

        fseek (f , 0 , SEEK_END);
        fileSize = ftell (f);
        rewind (f);
        return 1;
    }


    //Функция чтения блока из файла возвращает -1 когда достигнут конец файла
    //Возвпащает 1 если конец файла не достигнут
    int  ReadNextUDP()
    {

        char buf[BytesPerBlock];
        char*p;
        blocksNumber++;
        p = itoa(blocksNumber,buf);
        strcpy(fileInf,"");
        strcat(fileInf,p);
        strcat(fileInf," ");

        char a;
        int flg;
        for(int i = 0; i < BytesPerBlock; i++)
        {
            flg = fread(&a, sizeof(char), 1, f);
            if(flg == 0)
            {
                buf[i]='\0';
                cout<<endl<<"Last Block";
                strcat(fileInf,buf);
                showBlockInfo();

                return -1;

            }
            buf[i] = a;
        }

        strcat(fileInf,buf);

        showBlockInfo();
        return 1;
    }

  //Функция чтения блока из файла возвращает -1 когда достигнут конец файла
    //Возвпащает 1 если конец файла не достигнут
    int  ReadNextTCP()
    {
        char a;
        int flg;
        blocksNumber++;
        for(int i = 0; i < BytesPerBlock; i++)
        {
            flg = fread(&a, sizeof(char), 1, f);
            if(flg == 0)
            {
                fileInf[i]='\0';
                cout<<endl<<"Last Block";
                blockSendSize = strlen(fileInf);
                return -1;

            }
            fileInf[i] = a;
        }
        showBlockInfo();


        return 1;
    }

    //Функция чтения всего файла целиком (не используется)
  /*  char* ReadFile()
    {
        size_t result;
        f = fopen(fileName, "r");

        if (f == NULL)
        {
            perror ("Error opening file");
        }

        fseek (f , 0 , SEEK_END);
        fileSize = ftell (f);
        rewind (f);
        // allocate memory to contain the whole file:
        fileInf = (char*) malloc (sizeof(char)*fileSize);

        // copy the file into the buffer:
        result = fread (fileInf,1,fileSize,f);
        if (result != fileSize)
        {
            cout<<"Reading error";
            exit (3);
        }
        fclose (f);
        return 0;
    }
*/
    //Функция создание заголовка пакета
    void createHeader()
    {
        char buf[20];
        char*p;
        p = itoa(fileSize,buf);


        Header = new char[strlen(fileName)+strlen(p)+2];
        strcpy(Header ,fileName);
        strcat(Header," ");
        strcat(Header,buf);
        strcat(Header," ");

        HeaderSize = strlen(Header);

    }
    void showBlockInfo()
    {
        cout<<endl<<endl<<"*******Block info*********";
        cout<<endl<<"BLock number:"<<blocksNumber;
        cout<<endl<<"Block info:"<<fileInf;
        blockSendSize = strlen(fileInf);
        cout<<endl<<"Block send size:"<<blockSendSize;
        cout<<endl<<"**************************";
    }
    void showInfo()
    {
        cout<<endl<<endl<<"////////Results of Transfer////////////";
        cout<<endl<<"Filesize:"<<fileSize;
        cout<<endl<<"Filename:"<<fileName;
        cout<<endl<<"Header:"<<Header;
        cout<<endl<<"Header length:"<<HeaderSize;
        cout<<endl<<"Total blocks count"<<blocksNumber;
        cout<<endl<<"//////////////////////////////////////////////";

    }
};


struct information
{
    int bytesSended;
    int bytesRecieved;
    int totalBytesSended ;
    int totalBytesRecieved ;
    int fails;
    void showSRinfo()
    {
        cout<<endl<<"**************Bytes Send Recieved:***************";
        cout<<endl<<"Bytes Recieved :"<<bytesRecieved;
        cout<<endl<<"Bytes sended:"<<bytesSended;
        cout<<endl<<"Total Bytes Send:"<<totalBytesSended;
        cout<<endl<<"Total Bytes Recieved:"<<totalBytesRecieved;
        cout<<endl<<"Total fails:"<<fails;
        cout<<endl<<"*************************************************";
    }
    void inc()
    {
        totalBytesRecieved += bytesRecieved;
        totalBytesSended += bytesSended;
    }

    void clear()
    {
        totalBytesRecieved = 0;
        totalBytesSended = 0;
        fails = 0;
    }
}inf;

void startUDP( FileRW* file,sockaddr_in server,sockaddr_in client)
{
    int sock;
    sock=socket(PF_INET,SOCK_DGRAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    bind(sock,(sockaddr *)&client,sizeof(client));
    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval));
    inf.clear();
    char buf[1000];

    for(;;)
    {
        inf.bytesSended = sendto(sock, file->Header, file->HeaderSize,0,(sockaddr*)&server,sizeof(server));
        inf.bytesRecieved = recvfrom(sock,buf,1000,0,NULL,0);
        if(inf.bytesRecieved != -1)
        {
            break;
        }
        else
        {
            cout<<endl<<"Resending Header:"<<endl;
            inf.fails++;
        }
    }

    cout<<endl<<"Header Bytes Send:"<<inf.bytesSended;


    int flg = 1;
    while(flg != -1)
    {
        flg = file->ReadNextUDP();

        for(;;)
        {
            inf.bytesSended = sendto(sock, file->fileInf, file->blockSendSize,0,(sockaddr*)&server,sizeof(server));

            inf.bytesRecieved = recvfrom(sock,buf,1000,0,NULL,0);
            inf.inc();
            inf.showSRinfo();
            if(inf.bytesRecieved != -1)
            {
                break;
            }
            else
            {
                cout<<endl<<"Resending Data:"<<endl;
                inf.fails++;
            }
        }
    }
    file->showInfo();
    inf.showSRinfo();
}

void startTCP(FileRW* file,sockaddr_in addr)
{
    inf.clear();
    int sockfd = socket(AF_INET, SOCK_STREAM, 0 );
    if(sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }



    if((inf.bytesSended = write(sockfd, file->Header, file->HeaderSize)) < 0)
    {
        perror("Send Normal Data");
        close(sockfd);
        exit(3);
    }
    cout<<endl<<"Header Bytes Send:"<<inf.bytesSended;
    int flg = 1;
    while(flg != -1)
    {
        flg = file->ReadNextTCP();
        if((inf.bytesSended = write(sockfd, file->fileInf, file->blockSendSize)) < 0 )
        {
            perror("Send Normal Data");
            close(sockfd);
            sockfd = socket(AF_INET, SOCK_STREAM, 0 );

            while(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0);
            connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
            inf.bytesSended = write(sockfd, file->fileInf, file->blockSendSize);
        }
        inf.inc();
        inf.showSRinfo();

    }
    file->showInfo();
    inf.showSRinfo();
}

void startOOB(sockaddr_in addr)
{
    inf.clear();
    int sockfd = socket(AF_INET, SOCK_STREAM, 0 );
    if(sockfd < 0)
    {
        perror("socket");
        exit(1);
    }


    if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }


   if((inf.bytesSended = send(sockfd, "44", 2,MSG_OOB)) < 0)
    {
        perror("Send OOB Data");
        close(sockfd);
        exit(3);
    }
    cout<<endl<<"OOB Bytes Send:"<<inf.bytesSended;



}

int main(int argc, char**argv)
{


    char* port = new char[10];
    char* ip = new char[100];
    char* filename = new char[100];
    char mode ;

    if(argc <5)
    {
        cout<<"Too few arguments";
    }
    ip  = argv[1];
    port = argv[2];
    filename = argv[3];
    mode  = argv[4][0];
    cout<<ip<<port<<filename<<mode;


    struct sockaddr_in server, client={AF_INET,INADDR_ANY,INADDR_ANY};


    FileRW* file = new FileRW(filename);

    file->openFile();

    file->createHeader();

    memset(&server,0,sizeof(server));

    server.sin_family=AF_INET;
    server.sin_port=htons(atoi(port));
    server.sin_addr.s_addr=inet_addr(ip);

    switch(mode)
    {
        case'T':
            cout<<"Start sending in TCP Mode"<<endl;
            startTCP(file,server);
            break;
        case'U':
            cout<<"Start sending in UDP Mode"<<endl;
            startUDP(file,server,client);
            break;
        case'O':
            cout<<"OOB data send"<<endl;
            startOOB(server);
            break;

        default:
            cout<<endl<<"Error only O U T for different modes";
            break;
    }

    return 0;

}















