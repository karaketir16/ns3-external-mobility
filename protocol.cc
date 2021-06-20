#include "protocol.h"

#define VERIFY 0xB00F
size_t verify_s = 2;            // verify sector size
size_t package_type_s = 1;      // package type sector size
size_t package_number_s = 8;    // package number sector size
size_t header_s = verify_s + package_type_s + package_number_s;  // Total header size

Protocol::Protocol()
{

}

// little endian and IEEE 754
size_t helper(void *dest, void const * src, size_t len){
    uint8_t * tmp_src = (uint8_t*) src;
    uint8_t * tmp_dest = (uint8_t*) dest;
    for(size_t i = 0; i < len; i++){
        tmp_dest[i]  = tmp_src[i];
    }
    return len;
}


PackageType Protocol::decode(size_t len){
    // len smaller than 3, invalid data
    uint8_t * iterator = (uint8_t*) buffer;

    if(len < header_s){
        return PackageType::__MAX__UNDEFINED__;
    }

    uint16_t head;
    iterator += DECODE_HELPER(head, iterator);

    // head is not {VERIFY}, invalid data
    if(head != VERIFY){
        return PackageType::__MAX__UNDEFINED__;
    }

    iterator += DECODE_HELPER(package_number, iterator);

    uint8_t pkg_type_tmp;
    iterator += DECODE_HELPER(pkg_type_tmp, iterator);
    PackageType pkg_type = (PackageType) pkg_type_tmp;

    switch (pkg_type) {
    case Start:
        break;
    case Information:
        iterator += DECODE_HELPER(state, iterator);
        iterator += DECODE_HELPER(totalReceivedData, iterator);
        break;
    case PackageType::ACK:
        break;
    case PackageType::Position:

        iterator += DECODE_HELPER(x, iterator);
        iterator += DECODE_HELPER(y, iterator);
        iterator += DECODE_HELPER(z, iterator);

        break;

    case PackageType::__MAX__UNDEFINED__:
        break;
    }

    return pkg_type;
}

size_t Protocol::encode(PackageType pkg_type){
    // len smaller than 3, invalid data
    uint8_t * iterator = (uint8_t*) buffer;

    // Write VERIFY header
    uint16_t head = VERIFY;
    iterator += ENCODE_HELPER(head, iterator);

    iterator += ENCODE_HELPER(package_number, iterator);

    uint8_t pkg_type_tmp = (uint8_t) pkg_type;
    iterator += ENCODE_HELPER(pkg_type_tmp, iterator);

    switch (pkg_type) {
    case Start:
        break;
    case Information:
        iterator += ENCODE_HELPER(state, iterator);
        iterator += ENCODE_HELPER(totalReceivedData, iterator);
        break;
    case PackageType::ACK:
        break;
    case PackageType::Position:

        iterator += ENCODE_HELPER(x, iterator);
        iterator += ENCODE_HELPER(y, iterator);
        iterator += ENCODE_HELPER(z, iterator);

        break;

    case PackageType::__MAX__UNDEFINED__:
        break;
    }

    return iterator - (uint8_t*)buffer;
}
