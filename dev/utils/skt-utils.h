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
    
    void run_tcp_server(std::string host, size_t port, client_serve_fn fn);
    void run_tcp_client(std::string host, size_t port, client_serve_fn fn);
    void run_udp_server(std::string host, size_t port, const HOST_PORT_ARR& hp_arr, client_serve_fn fn);
    void run_udp_client(std::string host, size_t port, const host_port& cl_hp, client_serve_fn fn);    
}
