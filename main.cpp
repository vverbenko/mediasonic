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
                   /*
                    fprintf( ClientFP,
                    "<form action='test.htm' name='file' method='post'>\n"
                    "<input type='submit' value='Остановить сервер' title='' style='width:30%;'>\n"
                    "</form>\n");
                    fprintf( ClientFP,
                    "<form action='script' name='scr' method='post'>\n"
                    "<input type='submit' value='Воспроизвести трэк' title='' style='width:30%;'>\n"
                    "</form>\n");
                    fprintf( ClientFP, ХУЙ
                    "<form action='http://www.google.ru/search' method='get'  target='_blank'>\n"
                    "<input type='hidden' value='протокол http' name='q'>"
                    "<input type='submit' value='Отправить Google запрос на поиск' title='' style='width:30%;'>\n"
                    "</form>\n");
                    fprintf( ClientFP,
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
                    */

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




    // This shit may be useful
    // it's not for web clients
    // this is a server
    /*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int sock, listener;       // дескрипторы сокетов
    struct sockaddr_in addr; // структура с адресом
    char buf[2048];       // буфур для приема
    int bytes_read;           // кол-во принятых байт


    listener = socket(AF_INET, SOCK_STREAM, 0); // создаем сокет для входных подключений
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }

    // Указываем параметры сервера
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) // связываемся с сетевым устройство. Сейчас это устройство lo - "петля", которое используется для отладки сетевых приложений
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 1); // очередь входных подключений

    while(1)
    {
        sock = accept(listener, NULL, NULL); // принимаем входные подключение и создаем отделный сокет для каждого нового подключившегося клиента
        if(sock < 0)
        {
            perror("Прием входящих подключений");
            exit(3);
        }

switch(fork())
{
case -1:
 perror("fork");
 break;
case 0:
close(listener);


        while(1)
        {
            printf("Ожидаем сообщение...\n");


            bytes_read = recv(sock, buf, 2048, 0); // принимаем сообщение от клиента
            if(bytes_read <= 0) break;
            printf("Получено %d bytes\tСообщение: %s\n", bytes_read, buf);
            printf("Отправляю принятое сообщение клиенту\n");
            send(sock, buf, bytes_read, 0); // отправляем принятое сообщение клиенту
        }

        close(sock); // закрываем сокет
        _exit(0);
default:
close(sock);
}
    }

    return 0;
}
*/

    // this is a client
 /*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
char message[2048];//=(char*)malloc(sizeof(char));// сообщение на передачу
char buf[sizeof(message)];
int port,ch;
//----------------------------------------------------------------------------
if(argc!=3){
printf("Не верное кол-во аргументов!\nДолжно быть 2 аргумента (Порт,ip-адрес -сервера)!\n");
exit(0);
}

    int sock;                 // дескриптор сокета
    struct sockaddr_in addr; // структура с адресом
    struct hostent* hostinfo;
port = atoi(argv[1]);
hostinfo = argv[2];
    sock = socket(AF_INET, SOCK_STREAM, 0); // создание TCP-сокета
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    // Указываем параметры сервера
    addr.sin_family = AF_INET; // домены Internet
    addr.sin_port = htons(port); // или любой другой порт...
    addr.sin_addr.s_addr=inet_addr(hostinfo);
    /*
     * Начинающие программисты часто спрашивают, как можно отлаживать сетевую программу, если под рукой нет сети.
     * Оказывается, можно обойтись и без неё. Достаточно запустить клиента и сервера на одной машине, а затем использовать для соединения адрес интерфейса внутренней петли (loopback interface).
     * В программе ему соответствует константа INADDR_LOOPBACK (не забудьте применять к ней функцию htonl!).
     * Пакеты, направляемые по этому адресу, в сеть не попадают.
     * Вместо этого они передаются стеку протоколов TCP/IP как только что принятые.
     * Таким образом моделируется наличие виртуальной сети, в которой вы можете отлаживать ваши сетевые приложения.
     */
   // addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      //addr.sin_addr.s_addr = inet_addr(host_ip);

 /*
 if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) // установка соединения с сервером
    {
        perror("Подключение");
        exit(2);
    }

while(1){//WHILE <---
//----------------------------------------------------------------------------
    printf("Введите сообщение серверу(Для выхода:exit): ");
    if (!strcmp(gets(message), "exit")){close(sock);return 0;}
//----------------------------------------------------------------------------

    printf("отправка сообщения на сервер...\n");
    send(sock, message, sizeof(message), 0); // отправка сообщения на сервер
    int bytes_read = 0;
    printf("Ожидание сообщения\n");
    bytes_read = recv(sock, buf, sizeof(message), 0);
    printf("Получено %d bytes\tСообщение: %s\n", bytes_read, buf); // прием сообщения от сервера


}//END_WHILE

return 0;

}
*/

}








