#define BUFFERLEN 1024
#define NOPTHREAD

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#ifndef NOPTHREAD
#include <pthread.h>
#endif
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <errno.h>
#include <list>

using namespace std;

pthread_mutex_t mutex;
fstream file;

int recvFrom(int sock) {
    char buffer[BUFFERLEN];
    cout << "worker started" << endl; cout.flush();
    while (1) {
        pthread_mutex_lock(&mutex);
        int recvLen = recv (sock, buffer, BUFFERLEN, 0);
        if (recvLen <= 0) {
            close (sock);
            if(errno)
                std::cout << strerror(errno) << endl;
            cout << "worker finished" << endl; cout.flush();
	    pthread_mutex_unlock(&mutex);
            return 0;
        }
        else {
            cout << "received ";
            cout.write(buffer, recvLen);
            cout << endl;
            cout.flush();
            file.write(buffer, recvLen);
            file << endl;
            file.flush();
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

void* worker(void * arg ){
    recvFrom(*(int*)arg);
    return 0;
}

volatile bool g_stop;
void* cinReader(void*) {

    using namespace std;
    cout << "enter 'stop' to stop server" << endl; cout.flush();
    while(!g_stop) {
        string cmd;
        cin >> cmd;
        g_stop = (cmd=="stop");
    }

    return 0;
}

int main() {
    file.open("out.txt", ios_base::trunc | ios_base::out);
    pthread_mutex_init(&mutex, 0);
    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cout << strerror(errno) << endl; cout.flush();
        return errno;
    }

    unsigned short port = 0;
    cout << "bind port > "; cout.flush();
    cin >> port;
    sockaddr_in service;
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = 0;
    service.sin_port        = htons (port);

    if (bind (sock, (sockaddr*)&service, sizeof (service)) < 0) {
        close (sock);
        cout << strerror(errno) << endl; cout.flush();
        return errno;
    }

    if (listen (sock, 1) < 0) {
        close (sock);
        cout << strerror(errno) << endl; cout.flush();
        return errno;
    }

    g_stop = false;
    pthread_t cinReaderThread;
    pthread_create(&cinReaderThread, 0, cinReader,0);


#ifdef NOPTHREAD
    list<int> procList;
#else
    list<pthread_t> threadList;
#endif

    while(!g_stop) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int rez = select(sock+1, &fds, NULL, NULL, &timeout);
        if (rez < 0) {
            cout << strerror(errno) << endl; cout.flush();
            return errno;
        }
        else
            if ( FD_ISSET(sock, &fds))
            {
                int clientSock = accept (sock, NULL, NULL);

#ifdef NOPTHREAD
		cout << "forking" << endl; cout.flush();
                int proc = fork();
                if( proc == 0) {
                    recvFrom(clientSock);
                    return 0;
                }
                else {
                    procList.push_back(proc);
                }
#else
                cout << "starting thread" << endl; cout.flush();
                pthread_t thread;
                pthread_create(&thread, 0, worker, &clientSock);
                threadList.push_back(thread);
#endif
            }
    }
    pthread_join(cinReaderThread, 0);
#ifdef NOPTHREAD
    while(procList.size()) {
        kill(procList.front(), SIGTERM);
        procList.pop_front();
    }
#else
    while(threadList.size()) {
        pthread_join(threadList.front(), 0);
        threadList.pop_front();
    }
#endif
    file.close();
    return 0;
}
