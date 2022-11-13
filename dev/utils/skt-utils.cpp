#include "skt-utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

dev::ctf_packet::ctf_packet():
    frame_start(4),
    protocol(1),
    bdata_len(0),    
    seq(0)
{
};

dev::datalen_t dev::ctf_packet::setup_packet(datalen_t len)
{
    bdata_len = htons(len);
//    std::cout << "setup_packet of" << len << " " << bdata_len << std::endl;
    data[len] = ctf_frame_end;
    datalen_t rv = CTF_HEADER_SIZE + len + 1;
    return rv;
};

void dev::ctf_packet::setseq(seq_t s)const
{
    seq = htons(s);
};

std::ostream& dev::operator<<(std::ostream& os, const dev::ctf_packet& t)
{
    os << (int)t.frame_start << " " << t.protocol << " " << t.bdata_len << " len=" << ntohs(t.bdata_len) << " seq=" << ntohs(t.seq);
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
   ctf_socket
*/
void dev::ctf_socket::init(int sk)
{
    m_skt = sk;
};


/**
   blocking_tcp_socket
*/
bool dev::blocking_tcp_socket::send_packet(const ctf_packet& pkt, datalen_t wire_len)
{
    return sendall((char*)&pkt, wire_len);
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

std::pair<int, int>  dev::blocking_tcp_socket::read_packet(ctf_packet& pkt)
{
    std::pair<int, int> rv = {0,0};
    if(!readall((char*)&pkt, CTF_HEADER_SIZE))return {-1,-1};
    //std::cout << "in:" << pkt << std::endl;
    
    if(pkt.frame_start != ctf_frame_start)
    {
        std::cout << "invalid 'frame_start' received [" << (int)pkt.frame_start << "] expected [" << (int)ctf_frame_start << "]\n";
        return {0,0};
    }

    if(pkt.protocol != ctf_protocol)
    {
        std::cout << "invalid 'protocol' received [" << pkt.protocol << " expected [" << ctf_protocol << "]\n";
        return {0,0};
    }
    rv.first = ntohs(pkt.bdata_len);
    if(rv.first > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << rv.first
                  << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        rv.first = CTF_MAX_PAYLOAD;
        return {0,0};
    }
    
//    std::cout << "pkt:" << pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    if(!readall((char*)pkt.data, rv.first))return {-1,-1};
    char frame_end;
    if(!readall(&frame_end, 1))return {-1,-1};
    
    if(frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return {0,0};
    }

    return rv;    
};

/**
   udp_socket
*/
void dev::udp_socket::setConn(const dev::HOST_PORT_ARR& arr)
{
    m_conn_hp = arr;
};

bool dev::udp_socket::send_packet(const ctf_packet& pkt, datalen_t wire_len)
{
    return sendall((char*)&pkt, wire_len);
};

std::pair<int, int> dev::udp_socket::read_packet(ctf_packet& pkt)
{
    std::pair<int, int> rv = {0,0};
    if(!readall((char*)&pkt, sizeof(ctf_packet)))return {-1, -1};
    
    if(pkt.frame_start != ctf_frame_start)
    {
        std::cout << "invalid 'frame_start' received [" << (int)pkt.frame_start << "] expected [" << (int)ctf_frame_start << "]\n";
        return {0,0};
    }

    if(pkt.protocol != ctf_protocol)
    {
        std::cout << "invalid 'protocol' received [" << pkt.protocol << " expected [" << ctf_protocol << "]\n";
        return {0,0};
    }
    rv.first = ntohs(pkt.bdata_len);
    if(rv.first > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << rv.first << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        rv.first = CTF_MAX_PAYLOAD;
        return {0,0};
    }
    
    //std::cout << "pkt:" << m_pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    char frame_end = pkt.data[rv.first];
    if(frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return {0,0};
    }

    return rv;
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

        //if(n != 448){
        //    std::cout << len << " " << n << "\n";
        //}
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
