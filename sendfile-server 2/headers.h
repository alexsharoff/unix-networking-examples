#ifndef HEADERS_H
#define HEADERS_H

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <errno.h>
#include <list>
#include <sstream>
#include <map>

using namespace std;


list<pthread_t> threadList;

void createThread(void *(*start_routine) (void *),
                  void * arg = 0){
    pthread_t newThread;
    pthread_create(&newThread, 0, start_routine,arg);
    threadList.push_back(newThread);
}

void killAllThreads() {
    while(threadList.size()) {
        int rez;
        if ( (rez = pthread_cancel(threadList.front()) )< 0) {
            cout << strerror(rez) << endl; cout.flush();
        }
        threadList.pop_front();
    }
}

#endif // HEADERS_H
