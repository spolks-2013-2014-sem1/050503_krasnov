#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

int connfd;
int Summ = 0;

struct TXT;
class FileRW;

void Procent(int i,int n);
int MessageConvert (char* mesg,int buffSize,TXT &filename,TXT &fileContent,int &allsize);
void sig_urg(int);

struct TXT
{
    int size;
    char text[10000];

    void Write()
    {
        cout << "\n Input text: \n";
        cin >> text;
        size = strlen(text);
        cout << "\n len = " << size;
    }
};

class FileRW
{
    public:
    FileRW()
    {

    }

    // функция записи в файл, FileName - путь к файлу,
    // если файла нет он будет создан
    static void WriteFile(char* FileName,TXT txt)
    {
        FILE *f;
        f = fopen(FileName, "wb");

        for(int i = 0; i < txt.size; i++)
        {
            //if(text[i] == '\0') break;
            fwrite(&txt.text[i], sizeof(char), 1, f);
        }

        fclose(f);
    }

    // функция дозаписи в файл, FileName - путь к файлу
    static void AppendFile(char* FileName,TXT txt)
    {
        FILE *f;
        f = fopen(FileName, "a+");

        for(int i = 0; i < txt.size; i++)
        {
            //if(text[i] == '\0') break;
            fwrite(&txt.text[i], sizeof(char), 1, f);
        }

        fclose(f);
    }
    // функция чтения из файла, FileName - путь к файлу,
    static int ReadFile(char* FileName,TXT &txt)
    {
        FILE *f;
        f = fopen(FileName, "rb");

        if( f == NULL )
        {
            cout << "\n ERROR: can not open the file";
            return -1;
        }

        char a;
        int i = 0;

        while(fread(&a, sizeof(char), 1, f) != 0)
        {
            txt.text[i] = a;
            i++;
        }
        txt.text[i-1] = '\0';
        txt.size = i - 1;

        fclose(f);

        return 0;
    }

    /*  TXT txt; txt.Write();
    /   FileRW::WriteFile(s,txt);
    /   FileRW::ReadFile(s,txt);
    /                            */


};

//*
class Session
{
public:
    int procDescriptor;
    char fileName[100];

    int receivedByte;
    int COUNT;

    bool workState; // false - простой
                             // true  - идет прием

    int i;

    Session()
    {
        //workState = false;

    }

    Session(int ProcDescriptor)
    {
        procDescriptor = ProcDescriptor;
    }


    int SessionStart(int ProcDescriptor)
    {
        cout << "\n Session start";
        procDescriptor = ProcDescriptor;
        COUNT = 1000;
        char mesg[COUNT];
        int buffSize;

        int allSize;
        TXT fileName;
        TXT content;

        if(workState != true)
        {
            cout << "\n NEW WORK CYCLE!!!";

            workState = true;

            receivedByte = 0;
            Summ = 0;

            buffSize = recv(procDescriptor, mesg, COUNT, 0);

            cout << "\n buffSize: " << buffSize;

            if( buffSize <= 0)
            {
                cout << "-ERROR(RECV): process disconnect #" << procDescriptor << " buffSize = " << buffSize << endl;
                workState = false;
                return -1;
            }
            if(mesg == NULL)
            {
                cout << "\n-ERROR: mesg = NULL";
                workState = false;
                return -1;
            }
            if(strstr(mesg,"-exit"))
            {
                cout << "\n-process disconnected \n command: <-exit>";
                workState = false;
                return -1;
            }

            Summ += buffSize;

            if(MessageConvert(mesg,buffSize,fileName,content,allSize) == -1)
            {
                cout << "-ERROR: filename not found!!!";
                workState = false;
                return 0;
            }

            cout << "-process: #" << procDescriptor ;
            cout << " new Main message: " << endl;
            cout << " FileName: " << fileName.text << endl;
            cout << " AllSize:  " << allSize << endl;
            receivedByte += content.size;

            FileRW::WriteFile(fileName.text,content);
            cout << "\n-file: <" << fileName.text << "> create" << endl;
            cout << " ";

            cout << "\n receivedByte: " << receivedByte;
            cout << "\n allSize: " << allSize;

            i = 0;
        }
        else
        {
            cout << "\n ++++ RECONNECT ++++";
        }

        while (receivedByte < allSize)
        {
                    signal(SIGURG, sig_urg);
        fcntl(connfd, F_SETOWN, getpid());
            buffSize = recv(procDescriptor, mesg, COUNT, 0);

            if( buffSize <= 0)
            {
                cout << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++";
                cout << "\n-ERROR(RECV): process disconnect #" << procDescriptor;
                cout << "\n-to continue, you must reconnect";
                cout << "\n++++++++++++++++++++++++++++++++++++++++++++++++++++";
                return -1;
            }
            if(mesg == NULL)
            {
                cout << "\n-ERROR: mesg = NULL";
                return -1;
            }

            TXT buff;
            strcpy(buff.text,mesg);
            buff.size = buffSize;
            receivedByte += buffSize;

            FileRW::AppendFile(fileName.text,buff);

            //========================
            i++;
            cout << "\n=============================================================";
            cout << "\n NEW PART      #" << i;
            cout << "\n PART SIZE:    <" << buffSize << ">";
            if(buffSize != COUNT)
            {
                mesg[buffSize-1] = '\0';
                cout << "\n PART CONTENT: <" << mesg << ">";
            }
            else
            {
                cout << "\n PART CONTENT: <" << mesg << ">";
            }
            cout << "\n receivedByte: " << receivedByte;
            cout << "\n allSize: " << allSize;
            Procent(receivedByte,allSize);
            cout << "\n=============================================================";
        }
        workState = false;
        cout << "\n ++++++++++++++The End+++++++++++++";
        cout << "\n -report transactions:";
        cout << "\n -process:        #" << procDescriptor << ">";
        cout << "\n -filename:       <" << fileName.text << ">";
        cout << "\n -count part:     <" << i << ">";
        cout << "\n -receivedByte:   <" << receivedByte << ">";
        cout << "\n -allSize:        <" << allSize << ">";

        return 0;
    }
};//*/





int main()
{
    cout << "SERVER START" << endl;
    int port = 2000;

    Session ses;

    for(;;)
    {
        cout << "\n-port: " << port << endl;

        int listener;

        struct sockaddr_in serverAddres;
        struct sockaddr_in clientAddres;
        socklen_t clientSize;

        listener = socket(AF_INET,SOCK_STREAM,0);

        if (listener == -1)
        {
            cout << "-ERROR: creating socket" << endl;
            return 0;
        }

        cout << "-creating socket" << endl;

        serverAddres.sin_family = AF_INET;
        serverAddres.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddres.sin_port = htons(port);

        bind(listener,(struct sockaddr *)&serverAddres,sizeof(serverAddres));

        if(listen(listener,1024) == -1)
        {
            cout << "-ERROR: listening" << endl;
        }

        clientSize = sizeof(clientAddres);

        cout << "-start listening" << endl;
        connfd = accept(listener,(struct sockaddr *)&clientAddres,&clientSize);
        //close (listener);

        signal(SIGURG, sig_urg);
        fcntl(connfd, F_SETOWN, getpid());
        cout << "-stop listening" << endl;
        cout << "-connect to proc #" << connfd << endl;

        while(ses.SessionStart(connfd) == 0);

        close(connfd);
        port ++;
    }
}


int MessageConvert (char* mesg,int buffSize,TXT &filename,TXT &fileContent,int &allsize)
{
    int flg = 0;
    int j;
//===FILENAME============================================
    for( j = 0; j < buffSize; j++)
    {
        filename.text[j] = mesg[j];
        if(mesg[j] == '.')
        {
            flg = 1;
            break;
        }
    }

    if(flg == 0)
    {
        cout << "\n -ERROR: filename not found!!! ";
        cout << "\n size: <" << buffSize << ">";
        cout << "\n message: <" << mesg << ">";
        cout << "\n+++";
        return -1;
    }



    j++; filename.text[j] = mesg[j];
    j++; filename.text[j] = mesg[j];
    j++; filename.text[j] = mesg[j];
    j++; filename.text[j] = '\0';
    filename.size = strlen(filename.text);
    j++;

//====ALLSIZE==========================================
    TXT allsz;
    int i = 0;

    while(mesg[j] != ' ')
    {
        allsz.text[i++] = mesg[j++];
    }
    allsz.text[i] = '\0';
    allsz.size = strlen(allsz.text);
    j++;
//====TEXT===============================================
    i = 0;
    for( ; j < buffSize; j++)
    {
        fileContent.text[i++] = mesg[j];
    }
    fileContent.text[i] = '\0';

    fileContent.size = strlen(fileContent.text);

    cout << "\n filename    text: <" << filename.text << "> ";
    cout << "\n filename    size: <" << filename.size << "> ";
    cout << "\n fileContent size: <" << fileContent.size << "> ";
    cout << "\n fileContent text: <" << fileContent.text << "> ";
    cout << "\n allSize     size: <" << allsz.size << "> ";
    cout << "\n allSizeChar text: <" << allsz.text << "> ";
    allsize = atoi(allsz.text);
    cout << "\n allsize: <" << allsize << "> ";

    return 0;
}


void sig_urg(int signo)
{
    int n;
    char buff[100];
    cout << "\n <!!!>  expedited data  <!!!>";
    n = recv(connfd, buff, sizeof(buff)-1, MSG_OOB);
    buff[n] = '\0'; /* null terminate */
    cout << "\n -read n = " << n << " OOB byte: " << buff;
    cout << "\n -all the transmitted bytes: " << Summ;
    cout << "\n <!!!====================!!!>";
    cin>>n;
}



void Procent(int i,int n)
{
    cout << "\n Loading: ";
    double a = (i * 1.0) / n;
    a *= 50;
    int b = a;
    cout << b * 2 << "% [";
    for(int j = 0; j < 50; j++)
    {
        if(j <= b)  cout << "|";
        else        cout << " ";
    }
    cout << "]";
}







































