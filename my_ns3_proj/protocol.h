#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <unistd.h>
#include <stdint.h>

#define DECODE_HELPER(Destinaniton, Source) helper((void *)&Destinaniton, (void *)Source, sizeof (Destinaniton))
#define ENCODE_HELPER(Source, Destination) helper((void *)Destination, (void *)&Source, sizeof (Source))


#include "udpsockethelper.h"

enum PackageType {
    ACK = 0,
    Position,
    Start,
    Information,
    __MAX__UNDEFINED__
};

enum StateType {
    Waiting = 0,
    Running,
    End,
    __MAX__STATE__
};


// Alternative, cJson

class Protocol
{
public:
    Protocol();

    PackageType decode(size_t);
    size_t encode(PackageType pkg_type);

    char buffer[MAX_PKG_LEN];

    double x,y,z;
    uint64_t package_number;

    uint64_t totalReceivedData;

    uint8_t state;
};


// copy len byte from src to dest, return copied byte count
size_t helper(void *dest, void const *src, size_t len);

#endif // PROTOCOL_H