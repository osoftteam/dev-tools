#pragma once
#include "dev-utils.h"

#define RECEIVE_BUFF_SIZE 100000

namespace dev
{
    class ctf_messenger
    {
    public:
        bool init(int skt, size_t spin_sleep_time_ms);
        void spin_packets(char* bdata, uint32_t bdata_len);
        void receive_packets();
        void receive_fix_messages();
    private:
        bool send_packet(char* bdata, uint32_t bdata_len);
        int  read_packet();
        
        int        m_skt{-1};
        uint16_t   m_protocol{0};
        size_t     m_spin_sleep_time_ms{0};
        char       m_buf[RECEIVE_BUFF_SIZE];
    };
};
