#include "headers.h"
#include "tcp.h"
#include "udp.h"

int main() {
    createThread(tcpWorker);
    createThread(udpWorker);

    cout << "enter 'stop' to stop server" << endl; cout.flush();
    while(true) {
        string cmd;
        cin >> cmd;
        if (cmd=="stop") {
            //;_;
            killAllThreads();
            return 0;
        }
    }

    return 0;
}
