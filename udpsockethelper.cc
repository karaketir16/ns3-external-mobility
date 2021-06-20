#include "udpsockethelper.h"

#include <iostream>

UDPSocketHelper::UDPSocketHelper()
{

}

UDPSocketHelper::~UDPSocketHelper()
{
    if(sockfd){
        close(sockfd);
        sockfd = 0;
    }
}

bool UDPSocketHelper::Create()
{
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        return false;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    return true;
}

bool UDPSocketHelper::Bind(uint16_t port, std::string addr)
{
    // Filling server information
    servaddr.sin_family      = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr(addr.c_str());
    servaddr.sin_port        = htons(port);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 )
    {
        return false;
    }

    return true;
}

sockaddr_in UDPSocketHelper::SetDest(uint16_t port, std::string addr)
{
    // Filling receiver information
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = inet_addr(addr.c_str());;

    return cliaddr;
}

size_t UDPSocketHelper::RecvBlock(char *data)
{
    socklen_t len = sizeof(cliaddr);  //len is value/resuslt
    return recvfrom(sockfd, (char *)data, MAX_PKG_LEN,
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);
}

int UDPSocketHelper::RecvNotBlock(char *data, uint64_t ms)
{
    fd_set set;
    struct timeval timeout;


    /* Initialize the file descriptor set. */
    FD_ZERO (&set);
    FD_SET (sockfd, &set);

    /* Initialize the timeout data structure. */
    timeout.tv_sec = ms / 1000;
    timeout.tv_usec = (ms%1000) * 1000;

    /* select returns 0 if timeout, 1 if input available, -1 if error. */
    int res =  (select (FD_SETSIZE,
                        &set, NULL, NULL,
                        &timeout));
    socklen_t len = sizeof(cliaddr);  //len is value/resuslt

    if(res == 1){

        len = sizeof(cliaddr);  //len is value/resuslt

        return recvfrom(sockfd, (char *)data, MAX_PKG_LEN,
                        MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                        &len);
    }

    return -1;
}

size_t UDPSocketHelper::Send(char *data, size_t sz)
{
    return sendto(sockfd, (char *)data, sz,
           0, (const struct sockaddr *) &cliaddr,
           sizeof(cliaddr));
}

