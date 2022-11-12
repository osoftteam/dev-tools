#include "ctf-messenger.h"
#include "fix-utils.h"

dev::ctf_packet::ctf_packet():
    frame_start(4),
    protocol(8192),
    bdata_len(0),
    frame_end(3)
{
    // 32  -htos--> 8192
};

uint32_t dev::ctf_packet::setup_packet(uint32_t len)
{
    bdata_len = ntohl(len);
    data[len] = 3;
    uint32_t rv = len + 8;
    return rv;
};
