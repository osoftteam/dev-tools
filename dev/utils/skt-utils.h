#pragma once

#include "dev-utils.h"

#define CTF_MAX_PAYLOAD 16000

static constexpr char ctf_frame_start    {4};
static constexpr char ctf_frame_end      {3};
static constexpr uint16_t ctf_protocol   {32};

namespace dev
{
    #pragma pack (1)
    struct ctf_packet
    {
        char     frame_start;    //1
        uint16_t protocol;       //2
        uint32_t bdata_len;      //4
        char     data[CTF_MAX_PAYLOAD];
        char     frame_end;      //1

        ctf_packet();
        ///return size of data to be sent over wire
        uint32_t  setup_packet(uint32_t len);
    };
#pragma pack()
    
    using client_serve_fn = std::function<void(int)>;
    struct host_port
    {
        std::string host;
        size_t port;
    };
    using HOST_PORT_ARR = std::vector<host_port>;

    class blocking_tcp_socket
    {
    public:
        void init(int sk);
        int  read_packet(ctf_packet&);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
    private:
        int m_skt{-1};
    };

    class udp_socket
    {
    public:
        void init(int sk);
        void setConn(const dev::HOST_PORT_ARR& arr);
        int  read_packet(ctf_packet&);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);        
    protected:
        int                  m_skt{-1};
        dev::HOST_PORT_ARR   m_conn_hp;        
    };

    
    void run_tcp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_tcp_client(const host_port& conn_hp, client_serve_fn fn);
    void run_udp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_udp_client(const host_port& cl_bind_hp, client_serve_fn fn);

    std::ostream& operator<<(std::ostream& os, const dev::ctf_packet& t);
}
