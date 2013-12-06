#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <math.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#include <list>
#include <vector>

using namespace std;

//class Session;
class Client;
//void Session::listen(int lst);

//*

class String
{
    public:

    int size;
    char text[10000];

    String()
    {

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
    static void WriteFile(String FileName,String txt)
    {
        FILE *f;
        f = fopen(FileName.text, "wb");

        for(int i = 0; i < txt.size; i++)
        {
            //if(text[i] == '\0') break;
            fwrite(&txt.text[i], sizeof(char), 1, f);
        }

        fclose(f);
    }

    // функция дозаписи в файл, FileName - путь к файлу
    static void AppendFile(String FileName,String txt)
    {
        FILE *f;
        f = fopen(FileName.text, "a+");

        for(int i = 0; i < txt.size; i++)
        {
            //if(text[i] == '\0') break;
            fwrite(&txt.text[i], sizeof(char), 1, f);
        }

        fclose(f);
    }
    // функция чтения из файла, FileName - путь к файлу,
    static int ReadFile(char* FileName,String &txt)
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
};//*/



class Client
{
    public:

    int descriptor;
    bool resState;

    String message;
    String filename;

    int allSizeInt;
    int ResiveByte;

    Client()
    {
        descriptor = 0;
        resState = false;
    }
    Client(int desk)
    {
        descriptor = desk;
        resState = false;
    }

    int Resive()
    {
        cout << "\n Resive Start:";

        int count = 1000;
        char mesg[count];
        int size;

        size = recv(descriptor, mesg, count, 0);

        if(size <= 0)
        {
            cout << "\n =========================== ";
            cout << "\n = ERROR: (Recv size <= 0) = ";
            cout << "\n =========================== ";
            return -1;
        }

        strcpy(message.text,mesg);
        message.size = size;



        if(resState == false)
        {
            ResiveHeader();
        }
        else
        {
            ResiveBody();
        }

        return 0;
        /*
        cout << "\n =========================== ";
        cout << "\n process: <" << descriptor << ">";
        cout << "\n mesg[" << size << "]: <" << mesg << ">";
        cout << "\n mesgage[" << message.size << "]: <" << message.text << ">";

        cout << "\n =========================== ";//*/
    }

    void ResiveHeader()
    {
        if(convertHeader(message,filename,allSizeInt) == 0)
        {
            cout << "\n <<< ResiveHeader >>> ";
            cout << "\n filename:   <" << filename.text << ">";
            cout << "\n allSizeInt: <" << allSizeInt << ">";

            resState = true;
            ResiveByte = 0;
            String obj;
            obj.size = 0;
            strstr(obj.text,"");

            FileRW::WriteFile(filename, obj);

            ResiveBody();
        }
    }

    void ResiveBody()
    {
        String content;
        int part;
        //convertBody(message,part,content);

        cout << "\n <<< ResiveBody >>> ";
        //cout << "\n part:         <" << part << ">";
        cout << "\n content size: <" << message.size << ">";
        cout << "\n content:      <" << message.text << ">";

        ResiveByte += message.size;

        cout << "\n revise: [" << ResiveByte << "] from [" << allSizeInt << "]";

        FileRW::AppendFile(filename,message);

        if(ResiveByte >= allSizeInt)
        {
            resState = false;
            cout << "\n";
            cout << "\n +++ The end ++";
            cout << "\n filename: " << filename.text;
            cout << "\n allSize:  " << allSizeInt;
            cout << "\n +++++++++++++";
            cout << "\n";
            //strstr(filename," ");
        }
    }

    // разпоковывает сообщение на имя файла и общий размер
    int convertHeader(String Message,String &filename,int &allSizeInt)
    {
        int flg = 0;
        int j;

        for( j = 0; j < Message.size; j++)
        {
            if(Message.text[j] == ' ')
            {
                flg = 1;
                break;
            }
            filename.text[j] = Message.text[j];
        }

        if(flg == 0)
        {
            cout << "\n +\n Filename not found!!! \n +";
            return -1;
        }

        filename.text[++j] = '\0';
        filename.size = strlen(filename.text);

        String allSize;

        flg = 0;
        int i = 0;

        cout << "\n all size create: ";

        for( ; j < Message.size; ++j)
        {
            if(Message.text[j] == ' ')
            {
                flg = 1;
                break;
            }

            allSize.text[i++] = Message.text[j];
            cout << "\n (" << i << "): " << allSize.text[i];
        }

        allSize.text[i] = '\0';
        allSize.size = strlen(allSize.text);

        //*
        cout << "\n all content: <" << Message.text << ">";
        cout << "\n filename    text: " << filename.text;
        cout << "\n filename    size: " << filename.size;
        cout << "\n allSizeChar text: " << allSize.text;
        cout << "\n allSize     size: " << allSize.size;//*/
        //cin >> i;
        allSizeInt = atoi(allSize.text);
        //cout << "\n allsize: <" << allSizeInt << "> ";


        String newMessage;

        i = 0;
        while(Message.text[j] == '\n' || Message.text[j] == '\0' || Message.text[j] == ' ')j++;
        //j++;
        for( ; j < Message.size; j++)
        {
            newMessage.text[i++] = Message.text[j];
        }

        //strcpy(message.text, "                                                                           ");
        strcpy(message.text, newMessage.text);
        message.size = strlen(newMessage.text);



        return 0;
    }

    // разпоковывает сообщение на номер части и контент
    int convertBody(String Message,int &partNumber,String &content)
    {
        int flg = 0;
        int j;

        String part;

        for( j = 0; j < Message.size; j++)
        {
            if(Message.text[j] == ' ')
            {
                flg = 1;
                break;
            }
            part.text[j] = Message.text[j];
        }

        if(flg == 0)
        {
            cout << "\n +\n part not found!!! \n +";
            return -1;
        }

        part.text[j++] = '\0';
        part.size = strlen(part.text);

        int i = 0;
        for( ; j < Message.size; ++j)
        {
            content.text[i++] = Message.text[j];
        }

        content.text[i-1] = '\0';
        content.size = strlen(content.text);

        //*
        cout << "\n part        text: " << part.text;
        cout << "\n part        size: " << part.size;
        cout << "\n content     text: " << content.text;
        cout << "\n content     size: " << content.size;//*/

        partNumber = atoi(part.text);
        cout << "\n partNumber: <" << partNumber << "> ";

        return 0;
    }
};


class Session
{
    public:

    Session()
    {

    }

    void listen(int lst)
    {

        int listener;
        vector<Client> clientList;
        listener = lst;

        for(;;)
        {
            cout << "\n New iteration!!! ";

            fd_set rfds;
            FD_ZERO(&rfds);

            int max = 0;
            for(int i = 0; i < clientList.size(); i++)
            {
                FD_SET(clientList[i].descriptor, &rfds);
                if (max < clientList[i].descriptor)
                {
                    max = clientList[i].descriptor;
                }
            }

            FD_SET(listener, &rfds);

            if (max < listener)
            {
                max = listener;
            }

            cout << "\n listen select!!!\n";
            cout << "\n listen:";

            select(max + 1, &rfds, NULL, NULL, NULL);

            cout << "\n Yes!!!!";

            for(int i = 0; i < clientList.size(); i++)
            {
                if (FD_ISSET(clientList[i].descriptor, &rfds) != 0)
                {
                    int flgRes = clientList[i].Resive();
                    if(flgRes == -1)
                    {
                        clientList.erase(clientList.begin() + i);
                        i--;
                    }
                }
            }

            if(FD_ISSET(listener, &rfds) != 0)
            {
                struct sockaddr_in serverAddres;
                struct sockaddr_in clientAddres;
                socklen_t clientSize = sizeof(clientAddres);

                int connfd = accept(listener,(struct sockaddr *)&clientAddres,&clientSize);

                Client obj;
                obj.descriptor = connfd;

                clientList.insert(clientList.end(),obj);

                cout << "\n new connect:";
                cout << "\n process: #" << connfd;
            }
        }
    }
};



int main()
{
    cout << "\n-SERVER START-" << endl;
    int port = 2000;
    int connfd;


    cout << "\n -TCP SERVER START-" << endl;



    cout << "\n-port: " << port << endl;

    int listener;

    struct sockaddr_in serverAddres;
    struct sockaddr_in clientAddres;

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

    socklen_t clientSize = sizeof(clientAddres);

    cout << "-start listening" << endl;

    Session ses;
    ses.listen(listener);
    //gogogo(listener);
    //listen(listener);



}















