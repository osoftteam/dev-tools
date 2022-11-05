#include "skt-utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void dev::run_socket_server(std::string host, size_t port, client_serve_fn serve_fn)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    auto skt_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (skt_fd < 0) {
        perror("server/socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(skt_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &(address.sin_addr));
    address.sin_port = htons(port);

    if (bind(skt_fd, (struct sockaddr*)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while(true)
    {
        std::cout << "listen.. " << std::endl;
        //    check_read_partial_config_file();
        if (listen(skt_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        auto client_skt = accept(skt_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if(client_skt < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        static char client_ip[64] = "";
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        std::cout << "accepted client [" << client_ip << "]" << std::endl;

        if(serve_fn)
        {
            serve_fn(client_skt);
        }
        std::cout << "finished [" << client_ip << "]" << std::endl;
    }
};

void dev::run_socket_client(std::string host, size_t port, client_serve_fn serve_fn)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    auto skt_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (skt_fd < 0) {
        perror("client/socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(skt_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &(address.sin_addr));
    address.sin_port = htons(port);

    if (connect(skt_fd, (struct sockaddr *)&address, addrlen) < 0)
    {
        perror("connect");
        return;
    }

    static char client_ip[64] = "";
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
    std::cout << "connected [" << client_ip << "]" << std::endl;

    if(serve_fn)
    {
        serve_fn(skt_fd);
    }
};
