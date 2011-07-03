#ifndef TCP_H
#define TCP_H
#include "headers.h"
#define TCPPORT 9000

pthread_mutex_t tcpMutex;
int tcpSock;
int tcpCounter;

void* tcpRecvWorker(void* param) {
    fstream file;
    int id = tcpCounter;
    pthread_mutex_lock(&tcpMutex);
    {
        id = tcpCounter;
        cout << id << " start" << endl; cout.flush();
        stringstream ss;
        ss << "tcp" << id << ".out";
        file.open(ss.str().c_str(), ios_base::trunc | ios_base::out |
                  ios_base::binary);
        tcpCounter++;
    }
    pthread_mutex_unlock(&tcpMutex);


    int sock = *(int*)param;
    char buffer[1024*1024];
    bool more = true;
    while (more) {
        pthread_mutex_lock(&tcpMutex);
        int recvLen = recv (sock, buffer, sizeof(buffer), 0);
        if (recvLen <= 0) {
            close (sock);
            if(errno)
                std::cout << strerror(errno) << endl;
            pthread_mutex_unlock(&tcpMutex);
            more = false;
        }
        else {
            cout << id << " received "<< recvLen << " bytes" << endl;
            cout.flush();
            file.write(buffer, recvLen);
            file.flush();
        }
        pthread_mutex_unlock(&tcpMutex);
    }

    cout << id << " finished" << endl; cout.flush();
    file.close();
    return 0;
}

void* tcpWorker(void*) {
    tcpCounter = 1;
    pthread_mutex_init(&tcpMutex, 0);
    tcpSock = socket (AF_INET, SOCK_STREAM, 0);
    if (tcpSock < 0) {
        cout << strerror(errno) << endl; cout.flush();
        return &errno;
    }

    sockaddr_in service;
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = 0;
    service.sin_port        = htons (TCPPORT);

    if (bind (tcpSock, (sockaddr*)&service, sizeof (service)) < 0) {
        close (tcpSock);
        cout << strerror(errno) << endl; cout.flush();
        return &errno;
    }

    if (listen (tcpSock, 1) < 0) {
        close (tcpSock);
        cout << strerror(errno) << endl; cout.flush();
        return &errno;
    }
    while(true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(tcpSock, &fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int rez = select(tcpSock+1, &fds, NULL, NULL, &timeout);
        if (rez < 0) {
            cout << strerror(errno) << endl; cout.flush();
            return &errno;
        }
        else
            if ( FD_ISSET(tcpSock, &fds))
            {
                int clientSock = accept (tcpSock, NULL, NULL);
                createThread(tcpRecvWorker, &clientSock);
            }
    }

    return 0;
}

#endif // TCP_H
