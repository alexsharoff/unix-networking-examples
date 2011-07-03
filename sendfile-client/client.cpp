#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <errno.h>

using namespace std;


int main (int argc, char** argv) {
    if(argc < 4) {
        cout << "usage: ip port message" << endl;
        return 0;
    }
    int m_socket = socket (AF_INET, SOCK_STREAM, 0);
    if (m_socket < 0) {
        cout << strerror(errno) << endl;
        return errno;
    }

    sockaddr_in clientService;
    clientService.sin_family      = AF_INET;
    clientService.sin_addr.s_addr = inet_addr (argv[1]);
    clientService.sin_port        = htons (atoi(argv[2]));

    if (connect (m_socket, (sockaddr*)&clientService,
                 sizeof (clientService)) < 0) {
        close (m_socket);
        cout << strerror(errno) << endl;
        return errno;
    }


    int len = strlen(argv[3]);
    while(true) {
        send (m_socket, argv[3], len, 0);
        cout.write(argv[3], len);
        cout << " sent" << endl;
        cout.flush();

        usleep(10000);
    }
    close (m_socket);
    return 0;
}
