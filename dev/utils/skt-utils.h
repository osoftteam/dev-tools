#pragma once

#include "dev-utils.h"

#define CTF_HEADER_SIZE 8
#define CTF_MAX_PAYLOAD 16000

namespace dev
{
    using datalen_t = uint16_t;
    using seq_t = uint32_t;
    static constexpr char ctf_frame_start    {4};
    static constexpr char ctf_frame_end      {3};
    static constexpr uint16_t ctf_protocol   {1};
    
//    #pragma pack (1)
    struct ctf_packet
    {
        char              frame_start;    //1
        char              protocol;       //1
        datalen_t         bdata_len;      //2
        mutable seq_t     seq;            //4        
        char              data[CTF_MAX_PAYLOAD];

        ctf_packet();
        ///return size of data to be sent over wire
        datalen_t  setup_packet(datalen_t len);
        void       setseq(seq_t s)const;
    };
//#pragma pack()
    
    using client_serve_fn = std::function<void(int)>;
    struct host_port
    {
        std::string host;
        size_t port;
    };
    using HOST_PORT_ARR = std::vector<host_port>;

    class ctf_socket
    {
    public:
        void init(int sk);
    protected:
        int m_skt{-1};
    };
    
    class blocking_tcp_socket: public ctf_socket
    {
    public:
        bool send_packet(const ctf_packet& pkt, datalen_t wire_len);
        std::pair<int, int>  read_packet(ctf_packet&);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
    };

    class udp_socket: public ctf_socket
    {
    public:
        void setConn(const dev::HOST_PORT_ARR& arr);
        bool send_packet(const ctf_packet& pkt, datalen_t wire_len);
        std::pair<int, int>  read_packet(ctf_packet&);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);        
    protected:
        dev::HOST_PORT_ARR   m_conn_hp;        
    };

    
    void run_tcp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_tcp_client(const host_port& conn_hp, client_serve_fn fn);
    void run_udp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_udp_client(const host_port& cl_bind_hp, client_serve_fn fn);

    std::ostream& operator<<(std::ostream& os, const dev::ctf_packet& t);
}
