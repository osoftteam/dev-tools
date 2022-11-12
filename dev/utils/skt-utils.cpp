#include "skt-utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

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

std::ostream& dev::operator<<(std::ostream& os, const dev::ctf_packet& t)
{
    os << (int)t.frame_start << " " << t.protocol << " " << t.bdata_len << " " << t.frame_end;
    return os;
};


void dev::run_tcp_server(const host_port& bind_hp, client_serve_fn serve_fn)
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
    inet_pton(AF_INET, bind_hp.host.c_str(), &(address.sin_addr));
    address.sin_port = htons(bind_hp.port);

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

void dev::run_tcp_client(const host_port& conn_hp, client_serve_fn serve_fn)
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
    inet_pton(AF_INET, conn_hp.host.c_str(), &(address.sin_addr));
    address.sin_port = htons(conn_hp.port);

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

int bind_udp_socket(const dev::host_port& bind_hp)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    auto skt_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
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
    inet_pton(AF_INET, bind_hp.host.c_str(), &(address.sin_addr));
    address.sin_port = htons(bind_hp.port);

    if (bind(skt_fd, (struct sockaddr*)&address, addrlen) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return skt_fd;
};

void dev::run_udp_server(const host_port& bind_hp, client_serve_fn serve_fn)
{
    auto skt_fd = bind_udp_socket(bind_hp);
    if(skt_fd > 0)
    {
        if(serve_fn)
        {
            std::cout << "running udp svr on [" << bind_hp.host << ", " << bind_hp.port << "]" << std::endl;
            serve_fn(skt_fd);
        }
    }
};

void dev::run_udp_client(const host_port& cl_bind_hp, client_serve_fn client_fn)
{
    std::cout << "dev::run_udp_client " << cl_bind_hp.host << " " << cl_bind_hp.port << std::endl;
    auto skt_fd = bind_udp_socket(cl_bind_hp);

    if(skt_fd > 0)
    {
        if(client_fn)
        {
            std::cout << "running udp client on [" << cl_bind_hp.host << ", " << cl_bind_hp.port << "]" << std::endl;
            client_fn(skt_fd);
        }        
    }
};

/**
   blocking_tcp_socket
*/
void dev::blocking_tcp_socket::init(int sk)
{
    m_skt = sk;
};

bool dev::blocking_tcp_socket::sendall(char *buf, size_t len)
{    
    size_t total = 0;
    int bytesleft = len;
    int n = 0;
    while(total < len)
    {
       n = send(m_skt, buf+total, bytesleft, 0);
       if (n == -1) {
           perror("socket-send");
           return false;
       }
       total += n;
       bytesleft -= n;
    }
    return true;
};


bool dev::blocking_tcp_socket::readall(char *buf, size_t len)
{
    size_t total = 0;
    int bytesleft = len;
    int n = 0;
    while(total < len)
    {
        n = read(m_skt, buf+total, bytesleft);
        if (n == -1) {
            perror("socket-read");
            return false;
        }
        else if(n == 0){
            perror("socket-read/closed");
            return false;
        }
        total += n;
        bytesleft -= n;
    }
    return true;
};

int dev::blocking_tcp_socket::read_packet(ctf_packet& pkt)
{
    if(!readall((char*)&pkt, 7))return -1;
    
    if(pkt.frame_start != ctf_frame_start)
    {
        std::cout << "invalid 'frame_start' received [" << (int)pkt.frame_start << "] expected [" << (int)ctf_frame_start << "]\n";
        return 0;
    }
    auto protocol_h = ntohs(pkt.protocol);
    if(protocol_h != ctf_protocol)
    {
        std::cout << "invalid 'protocol' received [" << protocol_h << " expected [" << ctf_protocol << "]\n";
        return 0;
    }
    auto bdata_len_h = htonl(pkt.bdata_len);
    if(bdata_len_h > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << bdata_len_h << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        bdata_len_h = CTF_MAX_PAYLOAD;
    }
    
//    std::cout << "pkt:" << pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    if(!readall((char*)pkt.data, bdata_len_h))return -1;
    if(!readall((char*)&pkt.frame_end, 1))return -1;
    
    if(pkt.frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)pkt.frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return 0;
    }

    return bdata_len_h;    
};

/**
   udp_socket
*/
void dev::udp_socket::init(int sk)
{
    m_skt = sk;
};

void dev::udp_socket::setConn(const dev::HOST_PORT_ARR& arr)
{
    m_conn_hp = arr;
};

int dev::udp_socket::read_packet(ctf_packet& pkt)
{
    if(!readall((char*)&pkt, sizeof(ctf_packet)))return -1;
    
    if(pkt.frame_start != ctf_frame_start)
    {
        std::cout << "invalid 'frame_start' received [" << (int)pkt.frame_start << "] expected [" << (int)ctf_frame_start << "]\n";
        return 0;
    }
    auto protocol_h = ntohs(pkt.protocol);
    if(protocol_h != ctf_protocol)
    {
        std::cout << "invalid 'protocol' received [" << protocol_h << " expected [" << ctf_protocol << "]\n";
        return 0;
    }
    auto bdata_len_h = htonl(pkt.bdata_len);
    if(bdata_len_h > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << bdata_len_h << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        bdata_len_h = CTF_MAX_PAYLOAD;
    }
    
    //std::cout << "pkt:" << m_pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    
    if(pkt.frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)pkt.frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return 0;
    }

    return bdata_len_h;
};

bool dev::udp_socket::readall(char *buf, size_t len)
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    
    for(const auto& c : m_conn_hp)
    {
        inet_pton(AF_INET, c.host.c_str(), &(address.sin_addr));
        address.sin_port = htons(c.port);

        int n = recvfrom(m_skt, buf, len, 
                         MSG_WAITALL, (struct sockaddr *) &address,  
                         &addrlen);
        if (n == -1) {
            perror("udp-socket-read");
            return false;
        }
        else if(n == 0){
            perror("socket-read/closed");
            return false;
        }
    }
    
    return true;
};

bool dev::udp_socket::sendall(char *buf, size_t len)
{
    struct sockaddr_in address;
    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    
    for(const auto& c : m_conn_hp)
    {
        inet_pton(AF_INET, c.host.c_str(), &(address.sin_addr));
        address.sin_port = htons(c.port);
        
        int n = sendto(m_skt, buf, len, 
                       MSG_CONFIRM, (const struct sockaddr *) &address,  
                       sizeof(address));
        if (n == -1) {
            perror("udp-socket-send");
            return false;
        }        
    }
    
    return true;
};
