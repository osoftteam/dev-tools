#pragma once

#include "dev-utils.h"
#include "fix-utils.h"
#include "tag-generator.h"

#define CTF_HEADER_SIZE 8
#define CTF_MAX_PAYLOAD 16000

namespace dev
{
    enum class skt_protocol
    {
        tcp,
        udp,
        unix
    };
    
    using datalen_t = uint16_t;
    using seq_t = uint32_t;
    static constexpr char ctf_frame_start    {4};
    static constexpr char ctf_frame_end      {3};
    static constexpr uint16_t ctf_protocol   {1};

    class stat_tag_mapper;
    
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

    struct sck_pkt_res
    {
        int len;
        int seq;
    };
    
    using client_serve_fn = std::function<void(int)>;
    struct host_port
    {
        std::string host;
        size_t port;
    };
    using HOST_PORT_ARR = std::vector<host_port>;

    template<class B>
    class ctf_socket
    {
    public:
        dev::sck_pkt_res read_packet();
        
        void init(int sk)                                       {m_skt = sk;};
        void start_session()                                    {m_session_start = time(nullptr);};
        const ctf_packet& pkt()const                            {return m_pkt;}
        void set_parse_fix_tags(bool v)                         {m_parse_fix_tags = v;}
        void set_fix_stat_tag_mapper(dev::stat_tag_mapper* v)   {m_stat_tag_mapper = v;}
    protected:

        bool          process_packet_stat(const sck_pkt_res& r);
        
        int           m_skt{-1};
        ctf_packet    m_pkt;
        std::string   m_stat_str;
        size_t        m_session_start{0};
        size_t        m_pnum {0};
        size_t        m_total_received{0};
        size_t        m_print_time{0};
        size_t        m_prev_pkt_num{0};
        size_t        m_lost_pkt_num{0};
        bool          m_parse_fix_tags{false};
        stat_tag_mapper* m_stat_tag_mapper{nullptr};
    };
    
    class blocking_tcp_socket: public ctf_socket<blocking_tcp_socket>
    {
    public:
        bool send_packet(const ctf_packet& pkt, datalen_t wire_len);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
        sck_pkt_res read_packet_no_stat(ctf_packet&);
        void set_mcast_conn(const dev::HOST_PORT_ARR&){};
    };

    class udp_socket: public ctf_socket<udp_socket>
    {
    public:
        bool send_packet(const ctf_packet& pkt, datalen_t wire_len);
        bool readall(char *buf, size_t len);
        bool sendall(char *buf, size_t len);
        sck_pkt_res read_packet_no_stat(ctf_packet&);
        void set_mcast_conn(const dev::HOST_PORT_ARR& arr);
    protected:
        dev::HOST_PORT_ARR   m_mcast;
    };

    int bind_udp_socket(const dev::host_port& bind_hp);    
    void run_tcp_server(const host_port& bind_hp, client_serve_fn fn);
    void run_unix_socket_server(const host_port& bind_hp, client_serve_fn fn);
    void run_udp_server(const host_port& bind_hp, client_serve_fn fn);
    template<class CP>
    void run_tcp_client(const host_port& conn_hp, CP& processor);    
    template<class CP>
    void run_udp_client(const host_port& cl_bind_hp, CP& processor);
    template<class CP>
    void run_unix_client(const host_port& conn_hp, CP& processor);

    std::ostream& operator<<(std::ostream& os, const dev::ctf_packet& t);

    void showip(const char* host);
}

template<class B>
dev::sck_pkt_res dev::ctf_socket<B>::read_packet()
{
    auto r = static_cast<B*>(this)->read_packet_no_stat(m_pkt);
    if(r.len <= 0)return r;
    if(m_parse_fix_tags)
    {
        std::string_view s(m_pkt.data, r.len);
        dev::fixmsg_view f(s, m_stat_tag_mapper);
    }
        
    if(process_packet_stat(r))
    {
        std::cout << m_stat_str << std::endl;
    };
    return r;
};


template<class B>
bool dev::ctf_socket<B>::process_packet_stat(const sck_pkt_res& r)
{
    m_total_received += r.len;
    ++m_pnum;

    if(r.seq != 0)
    {
        if(r.seq != (int)m_prev_pkt_num + 1)
        {
            ++m_lost_pkt_num;
        }
    }
    m_prev_pkt_num = r.seq;
        
    auto now = time(nullptr);
    auto time_delta = now - m_session_start;
    if(m_print_time != (size_t)time_delta && time_delta%2 == 0)
    {
        m_print_time = (size_t)time_delta;
        auto bpsec = int(m_total_received / time_delta);
        std::ostringstream out;
        out << "#" << m_pnum << " ";
        if(m_lost_pkt_num > 0){
            out << " lost=" << m_lost_pkt_num;
            auto lost_perc = 100.0 * m_lost_pkt_num/m_pnum;
            std::ostringstream out2;
            out2.precision(2);
            out2 << std::fixed << lost_perc;
            out << "(" << out2.str() << "%)";
        }
        //out << dev::size_human(m_total_received) << " "
        out << dev::size_human(bpsec) << "/sec ";

        if(m_parse_fix_tags)
        {
            auto msg_psec = int(m_pnum / time_delta);
            out << dev::size_human(msg_psec, false) << "msg/sec ";
            if(m_stat_tag_mapper)
            {
                for(const auto& i : m_stat_tag_mapper->stat())
                {
                    out << i.first << "=";
                    std::visit([&out](auto&& st){out << st;}, i.second);
                    out << "  ";
                }            
            }
        }
        
        m_stat_str = out.str();
        return true;
    }
    return false;
};

//////////////// runners //////////////////
template<class CP>
void dev::run_tcp_client(const host_port& conn_hp, CP& processor)
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

    processor(skt_fd);
};

template<class CP>
void dev::run_udp_client(const host_port& cl_bind_hp, CP& processor)
{
    std::cout << "dev::run_udp_client " << cl_bind_hp.host << " " << cl_bind_hp.port << std::endl;
    auto skt_fd = bind_udp_socket(cl_bind_hp);
    processor(skt_fd);
};

template<class CP>
void dev::run_unix_client(const host_port& conn_hp, CP& processor)
{
    std::cout << "running unix socket client [" << conn_hp.host << "]\n";
    
    struct sockaddr_un address;
    auto skt_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (skt_fd < 0) {
        perror("server/unix socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0 , sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, conn_hp.host.c_str() );
    auto len = strlen(address.sun_path) + sizeof(address.sun_family);
    if (connect(skt_fd, (struct sockaddr *)&address, len) < 0)
    {
        perror("unix socket connect");
        return;
    }
    std::cout << "connected\n";

    processor(skt_fd);
};
