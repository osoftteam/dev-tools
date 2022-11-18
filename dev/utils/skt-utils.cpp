#include "skt-utils.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

void dev::run_unix_socket_server(const host_port& bind_hp, client_serve_fn serve_fn)
{
    std::cout << "running unix socket server [" << bind_hp.host << "]\n";
    
    struct sockaddr_un address, remote;
    auto skt_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    if (skt_fd < 0) {
        perror("server/unix socket failed");
        exit(EXIT_FAILURE);
    }


    memset(&address, 0 , sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, bind_hp.host.c_str() );
	unlink(address.sun_path);
    auto len = strlen(address.sun_path) + sizeof(address.sun_family);
    if (bind(skt_fd, (struct sockaddr*)&address, len) < 0) {
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

        auto client_skt = accept(skt_fd, (struct sockaddr*)&remote, (socklen_t*)&len);
        if(client_skt < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }


        std::cout << "accepted client" << std::endl;

        if(serve_fn)
        {
            serve_fn(client_skt);
        }
        std::cout << "finished" << std::endl;
    }
};



int dev::bind_udp_socket(const dev::host_port& bind_hp)
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


dev::sck_pkt_res dev::blocking_tcp_socket::read_packet_no_stat(ctf_packet& pkt)
{
    dev::sck_pkt_res rv = {0,0};
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
    rv.len = ntohs(pkt.bdata_len);
    if(rv.len > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << rv.len
                  << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        rv.len = CTF_MAX_PAYLOAD;
        return {0,0};
    }
    rv.seq = ntohs(pkt.seq);
    
//    std::cout << "pkt:" << pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    if(!readall((char*)pkt.data, rv.len + 1))return {-1,-1};
    char frame_end = pkt.data[rv.len];
//    if(!readall(&frame_end, 1))return {-1,-1};    
    if(frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return {0,0};
    }
    pkt.data[rv.len] = 0;
    
    return rv;    
};

/**
   udp_socket
*/
void dev::udp_socket::set_mcast_conn(const dev::HOST_PORT_ARR& arr)
{
    m_mcast = arr;
};

bool dev::udp_socket::send_packet(const ctf_packet& pkt, datalen_t wire_len)
{
    return sendall((char*)&pkt, wire_len);
};


dev::sck_pkt_res dev::udp_socket::read_packet_no_stat(ctf_packet& pkt)
{
    sck_pkt_res rv = {0,0};
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
    rv.len = ntohs(pkt.bdata_len);
    if(rv.len > CTF_MAX_PAYLOAD)
    {
        std::cout << "big data size received [" << rv.len << " payload limited to [" << CTF_MAX_PAYLOAD << "]\n";
        rv.len = CTF_MAX_PAYLOAD;
        return {0,0};
    }
    rv.seq = ntohs(pkt.seq);
    
    //std::cout << "pkt:" << m_pkt << " protocol=" << protocol_h << " len=" << bdata_len_h << std::endl;
    char frame_end = pkt.data[rv.len];
    if(frame_end != ctf_frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)frame_end << "] expected [" << (int)ctf_frame_end << "]\n";
        return {0,0};
    }
    pkt.data[rv.len] = 0;
    
    return rv;
};

bool dev::udp_socket::readall(char *buf, size_t len)
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    memset(&address, 0 , sizeof(address));
    address.sin_family = AF_INET;
    
    for(const auto& c : m_mcast)
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
    
    for(const auto& c : m_mcast)
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

///
/// code taken from https://beej.us/guide/bgnet/html/#blocking
/// original: showip.c
///
void dev::showip(const char* host)
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return;
    }

    printf("IP addresses for %s:\n\n", host);

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        std::string ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver.c_str(), ipstr);
    }

    freeaddrinfo(res);

};
