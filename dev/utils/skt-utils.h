#pragma once

#include "dev-utils.h"

namespace dev
{
    using client_serve_fn = std::function<void(int)>;
    
    void run_socket_server(std::string host, size_t port, client_serve_fn fn);
    void run_socket_client(std::string host, size_t port, client_serve_fn fn);
}
