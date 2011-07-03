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
        cout << "usage: ip port filename [-u]" << endl;
        return 0;
    }
    bool udp = false;
    if(argc == 5)
        udp = true;


    int m_socket;
    if(udp)
        m_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    else
        m_socket = socket (AF_INET, SOCK_STREAM, 0);
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
    ifstream file;
    file.open(argv[3], ios_base::in | ios_base::binary);
    if(!file.is_open()) {
        cout << "file not found" << endl;
        return 0;
    }
    file.seekg(0, ios::end);
    int length = file.tellg();
    file.seekg(0, ios::beg);

    int total = 0;
    cout << "sending " << argv[3] << " to " << argv[1] << ":" << argv[2] << endl;
    while(1) {
        char buffer[1024];
        int readLen = file.readsome(buffer, sizeof(buffer));
        total += readLen;
        if(readLen > 0) {
            int sent = 0;
            while(sent != readLen) {
                int rez = send (m_socket, buffer + sent, readLen-sent, 0);
                if(rez < 0) {
                    cout << strerror(errno) << endl;
                    return errno;
                }
                sent += rez;
            }
        }
        else
            break;
        cout << total << "/" <<length << endl;

        if(udp) {
            recv(m_socket, buffer, sizeof(buffer), 0);
        }
        usleep(1000);
    }
    close (m_socket);
    file.close();
    return 0;
}
