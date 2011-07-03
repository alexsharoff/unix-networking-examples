#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <errno.h>
#include <vector>


int main() {
    using namespace std;

    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cout << strerror(errno) << endl; cout.flush();
        return errno;
    }

    unsigned short port = 8182;
    cout << "bind port > "; cout.flush();
    cin >> port;
    sockaddr_in service;
    service.sin_family      = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
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

    bool stop = false;
    vector<int> sockets;
    fstream file;
    file.open("out.txt", ios_base::trunc | ios_base::out);



    while(!stop) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        int maxDesc = sock;
        for(vector<int>::iterator it = sockets.begin();
            it != sockets.end(); it++) {
            FD_SET(*it, &fds);
            if(*it > maxDesc) maxDesc = *it;
        }
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int rez = select(maxDesc+1, &fds, NULL, NULL, &timeout);
        if (rez < 0) {
            cout << strerror(errno) << endl; cout.flush();
            return errno;
        }
        else {
            for(int i = 0; i < sockets.size(); i++){
                if(FD_ISSET(sockets[i], &fds)){
                    char buffer[1024];
                    int recvLen = recv (sockets[i], buffer, sizeof(buffer), 0);
                    if(recvLen == EAGAIN)
                        continue;
                    if (recvLen <= 0) {
                        close (sockets[i]);
                        if(errno)
                            cout << strerror(errno) << endl;
                        cout << sockets[i] << " closed" << endl; cout.flush();
                        sockets.erase(sockets.begin()+i);
                        i--;
                    }
                    else {
                        cout << "from " << sockets[i] << ": ";
                        cout.write(buffer, recvLen);
                        cout << endl;
                        cout.flush();
                        file.write(buffer, recvLen);
                        file << endl;
                        file.flush();
                    }
                }
            }
            if ( FD_ISSET(sock, &fds)){
                sockets.push_back(accept (sock, NULL, NULL));
            }
        }
    }
    file.close();
    return 0;
}
