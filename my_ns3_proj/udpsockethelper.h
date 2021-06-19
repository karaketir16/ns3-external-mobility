#ifndef UDPSOCKETHELPER_H
#define UDPSOCKETHELPER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#define MAX_PKG_LEN 1024

class UDPSocketHelper
{
public:
    UDPSocketHelper();
    ~UDPSocketHelper();
    bool Create();
    bool Bind(uint16_t port, std::string addr = "0.0.0.0");
    sockaddr_in SetDest(uint16_t port, std::string addr = "0.0.0.0");
    size_t RecvBlock(char *data);
    int RecvNotBlock(char *data, uint64_t ms);
    size_t Send(char *data, size_t sz);

//private:
    int sockfd = 0;
    struct sockaddr_in servaddr, cliaddr;
};

#endif // UDPSOCKETHELPER_H
