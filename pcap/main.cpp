#include <pcap/pcap.h>
#include <iostream>
#include <stdint.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <sstream>
#include <cstring>
using namespace std;

int main(int argc, char** argv)
{
    char errbuf[1024];
    if(argc < 3)
    {
        cout << "usage: lab5 <dev_name> <port>" << endl;
        cout.flush();
        return 0;
    }

    pcap_t* session = pcap_open_live(
                argv[1], BUFSIZ, 0, -1, errbuf);
    if(session == NULL)
    {
        cout << "pcap_open_live(): " << errbuf << endl;
        cout.flush();
        return 0;
    }

    stringstream ss;
    ss <<"tcp dst port " << argv[2];
    bpf_program filter;
    int rez = pcap_compile(session, &filter,
                           ss.str().c_str(), 0,0);
    if(rez < 0)
    {
        cout << "pcap_compile error" << endl;
        cout.flush();
        return 1;
    }

    rez = pcap_setfilter(session, &filter);
    if(rez < 0)
    {
        cout << "pcap_setfilter error" << endl;
        cout.flush();
        return 1;
    }
    while(true)
    {
        pcap_pkthdr* info;
        const uint8_t* raw_packet;
        int rez = pcap_next_ex(session, &info, &raw_packet);
        if(rez > 0)
        {
            ip* iph = (ip*)(raw_packet + sizeof(ether_header));
            tcphdr* tcph = (tcphdr*)(raw_packet + sizeof(ether_header) +
                                     iph->ip_hl * 4);
            char *msg = (char*)(raw_packet +
                                                  sizeof(ether_header) +
                                                  iph->ip_hl * 4 +
                                                  tcph->doff * 4);

            if(strlen(msg) > 0)
            {
                cout << msg;
                if(strstr(msg, "USER")){
                    cout << "USER command found" << endl;
                }
                cout.flush();
            }
        }
        if(rez == -1)
        {
            cout << "capture error" << endl;
            cout.flush();
        }
    }
    return 0;
}
