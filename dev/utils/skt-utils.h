#pragma once

#include "dev-utils.h"

namespace dev
{
    using client_serve_fn = std::function<void(int)>;
    struct host_port
    {
        std::string host;
        size_t port;
    };
    using HOST_PORT_ARR = std::vector<host_port>;

    class blocking_tcp_socket
    {
        int m_skt{-1};
    public:
        void init(int sk);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
    };

    class udp_socket
    {
        int                  m_skt{-1};
        dev::HOST_PORT_ARR   m_conn_hp;
    public:
        void init(int sk);
        void setConn(const dev::HOST_PORT_ARR& arr);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
    };

    
    void run_tcp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_tcp_client(const host_port& conn_hp, client_serve_fn fn);
    void run_udp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_udp_client(const host_port& cl_bind_hp, client_serve_fn fn);    
}
