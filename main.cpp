#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdarg.h>
#include <stdlib.h>

#define MAXBUF	10000
#define	PORT		9999
#define MAXPATH	255
#define MY_SOCK_PATH "/"

using namespace std;

char buffer[MAXBUF];

void myError(const char *err)
{
    perror(err);
    cerr << "Сервер остановлен" << endl;
}

int main()
{
    struct sockaddr_in addr;
    int sd, addrlen = sizeof(addr);
    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
    {
        myError("Socket");
        return 0;
    }

    int one = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(one));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if ( bind(sd, (struct sockaddr*)&addr, addrlen) < 0 )
    {
        myError("Bind");
        return 0;
    }

    if ( listen(sd, 5) < 0 )
    {
        myError("Listen");
        return 0;
    }

    cerr << "Веб сервер запущен" << endl;

    while (1)
    {

        int len;
        int client = accept(sd, NULL, NULL);

        if ( (len = recv(client, buffer, MAXBUF, 0)) > 0 )
        {
            FILE* ClientFP = fdopen(client, "w");
            if ( ClientFP == NULL )
                perror("fpopen");
            else
            {
                // Получить тип запроса
                char Req[MAXPATH];
                sscanf(buffer, "%s /", Req);
                // Если тип запроса - POST
                if (strcmp(Req, "POST") == 0) //strcmp - функция сравнения двух переменных формата Char
                {
                   // Обработка пути
                   char fname[MAXPATH];
                   char dirpath[MAXPATH];
                   sscanf(buffer, "POST %s HTTP", fname);
                   getcwd(dirpath, MAXPATH);

                   //cout << getcwd(dirpath, MAXPATH) << endl;


                   // Формирование пути к скрипту
                  // string cmd = "sh ";
                  // cmd += dirpath;
                  // cmd += fname;

                   //cout << cmd << endl;

                   if (strcmp(fname, "/script") == 0)
                   {
                        // Запуск скрипта
                        system("mocp -p");
                        cerr << "Получена команда воспроизвести трэк" << endl;

                   }


                   else if (strcmp(fname, "/test.htm") == 0)
                   {
                       cerr << "Получена команда остановить сервер" << endl;
                       system("mocp -s");
                       break;

                    }


                 }
                // Если тип запроса - GET
                else if (strcmp(Req, "GET") == 0)
                {
                    // Сообщаем серверу, что нужно будет интерпретировать
                    fprintf( ClientFP,
                    "HTTP/1.1 200  Ok\x0D\x0A"
                    "Server: webinfo\x0D\x0A"
                    "Content-Type: text/html; charset=utf-8\x0D\x0A"
                    "Connection: close"
                    "\x0D\x0A"
                    "\x0D\x0A"
                    );

                    // Считать содержимое индексной страницы и вернуть клиенту
                    string Page2Buffer;
                    ifstream IndexFile("index.html");
                    if (IndexFile.is_open())
                    {
                        while (!IndexFile.eof())
                        {
                            string line;
                            getline(IndexFile, line);
                            Page2Buffer.append(line + "\n");
                        }
                        IndexFile.close();
                        fwrite(Page2Buffer.c_str(), strlen(Page2Buffer.c_str()), 1, ClientFP);
                    }
                    cerr << "Клиент запросил index.html"<< endl;
                }
            fclose(ClientFP);
            }
        }
        close(client);
    }

    shutdown(sd, SHUT_RDWR);
    close(sd);
    cerr << "Работа сервера завершена" << endl;
    return 0;
}








