#ifndef UDP_H
#define UDP_H
#include "headers.h"
#define UDPPORT 9001

map<uint64_t,fstream*> socketStreamMap;

int udpSock;

void* udpWorker(void*) {
    int udpSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSock < 0) {
        cout << strerror(errno) << endl; cout.flush();
        return &errno;
    }

    sockaddr_in service;
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = 0;
    service.sin_port        = htons (UDPPORT);

    if (bind (udpSock, (sockaddr*)&service, sizeof (service)) < 0) {
        close (udpSock);
        cout << strerror(errno) << endl; cout.flush();
        return &errno;
    }

    while(true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(udpSock, &fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int rez = select(udpSock+1, &fds, NULL, NULL, &timeout);
        if (rez < 0) {
            cout << strerror(errno) << endl; cout.flush();
            return &errno;
        }
        else
            if ( FD_ISSET(udpSock, &fds))
            {
                char buffer[1024*1024];
                sockaddr_in client;
                socklen_t sockLen = sizeof(client);
                int recvLen = recvfrom(udpSock, buffer, sizeof(buffer),
                                       0,(sockaddr*)&client, &sockLen);

                uint64_t hash = (uint64_t)client.sin_addr.s_addr +
                        ((uint64_t)client.sin_port << 32);
                if(socketStreamMap.find(hash) == socketStreamMap.end()) {
                    cout << hash << " start" << endl; cout.flush();
                    stringstream ss;
                    ss << "udp" << hash << ".out";
                    fstream* file = new fstream();
                    file->open(ss.str().c_str(), ios_base::trunc | ios_base::out |
                               ios_base::binary);
                    socketStreamMap[hash] = file;
                }

                if (recvLen <= 0) {
                    if(errno)
                        std::cout << strerror(errno) << endl;
                }
                else {
                    int sent = 0;
                    while(sent != recvLen) {
                        int rez = sendto(udpSock, buffer, recvLen, 0,(sockaddr*)&client, sockLen);
                        if(rez < 0) {
                            cout << strerror(errno) << endl;
                        }
                        sent += rez;
                    }


                    cout << hash << " received "<< recvLen << " bytes"<< endl;
                    cout.flush();
                    (*socketStreamMap[hash]).write(buffer, recvLen);
                    (*socketStreamMap[hash]).flush();
                }
            }
    }
    return 0;
}

#endif // UDP_H
