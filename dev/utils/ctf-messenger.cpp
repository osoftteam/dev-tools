#include <arpa/inet.h>
#include <thread>
#include "ctf-messenger.h"
#include "fix-utils.h"

extern bool print_all_data;
extern bool collect_statistics;

static char frame_start    {4};
static char frame_end      {3};
static uint16_t protocol   {32};

bool dev::ctf_messenger::init(int skt, size_t sleep_time)
{
    m_skt = skt;
    m_protocol = htons(protocol);
    m_spin_sleep_time_ms = sleep_time;
    return true;
};

bool dev::ctf_messenger::send_packet(char* bdata, uint32_t bdata_len)
{
    if(!sendall(m_skt, (char*)&frame_start, 1))return false;
    if(!sendall(m_skt, (char*)&m_protocol, 2))return false;
    uint32_t bdata_len_h = ntohl(bdata_len);
    if(!sendall(m_skt, (char*)&bdata_len_h, 4))return false;
    if(!sendall(m_skt, (char*)bdata, bdata_len))return false;
    if(!sendall(m_skt, (char*)&frame_end, 1))return false;
    return true;
};

void dev::ctf_messenger::receive_packets()
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_received = 0;
    size_t print_time = 0;
    
    while(1)
    {
        auto bdata_len = read_packet();
        if(bdata_len <= 0)return;
        total_received += bdata_len;
        ++pnum;

        if(print_all_data){
            m_buf[bdata_len+1] = 0;
            std::cout << m_buf << std::endl;
        }
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;
        if(print_time != (size_t)time_delta && time_delta%2 == 0)
        {
            print_time = (size_t)time_delta;
            auto bpsec = int(total_received / time_delta);
            std::cout << "#" << pnum << " " << dev::size_human(total_received) << " " << dev::size_human(bpsec) << "/sec" << std::endl;
        }
    }
};

class stat_tag_mapper
{
    dev::T2STAT& m_stat;
public:
    stat_tag_mapper(dev::T2STAT& stat):m_stat(stat){}
    
    bool map_tag(size_t tag, const std::string_view& sv)
        {
            auto i = m_stat.find(tag);
            if(i != m_stat.end())
            {
                std::visit([&sv](auto&& st){st.update_stat(sv);}, i->second);
            }
            
            return true;
        };
};

/*
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;
*/

void dev::ctf_messenger::receive_fix_messages(T2STAT& stat)
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_received = 0;
    size_t print_time = 0;

    stat_tag_mapper tm(stat);
    auto tm_ptr = collect_statistics ? &tm : nullptr;
    
    while(1)
    {
        auto bdata_len = read_packet();
        if(bdata_len <= 0)return;
        std::string_view s(m_buf, bdata_len);
        dev::fixmsg_view f(s, tm_ptr);
        
        if(print_all_data){
            std::cout << s << std::endl;
        }
        
        total_received += bdata_len;
        ++pnum;
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;
        if(print_time != (size_t)time_delta && time_delta%2 == 0)
        {
            print_time = (size_t)time_delta;
            auto bpsec = int(total_received / time_delta);
            auto ppsec = int(pnum / time_delta);
            std::cout << "#" << pnum << " "
                      << dev::size_human(ppsec, false) << "msg/sec " << " "
                      << dev::size_human(bpsec) << "/sec ";

            if(collect_statistics)
            {
                for(const auto& i : stat)
                {
                    std::cout << i.first << "=";
                    std::visit([](auto&& st){std::cout << st;}, i.second);
                    std::cout << "  ";
                }
            }           
            std::cout << std::endl;
        }
    }
};

int dev::ctf_messenger::read_packet()
{
    char frame_start1{0};
    char frame_end1{0};
    uint16_t protocol1{0};
    if(!dev::readall(m_skt, (char*)&frame_start1, 1))return -1;
    if(frame_start1 != frame_start)
    {
        std::cout << "invalid 'frame_start' received [" << (int)frame_start1 << " expected [" << frame_start << "]\n";
        return -1;
    }
    if(!dev::readall(m_skt, (char*)&protocol1, 2))return -1;
    auto protocol_h = ntohs(protocol1);
    if(protocol_h != protocol)
    {
        std::cout << "invalid 'protocol' received [" << protocol_h << " expected [" << m_protocol << "]\n";
        return -1;
    }    
    uint32_t bdata_len;
    if(!dev::readall(m_skt, (char*)&bdata_len, 4))return -1;
    auto bdata_len_h = htonl(bdata_len);
    if(bdata_len_h > RECEIVE_BUFF_SIZE)
    {
        std::cout << "big data size received [" << bdata_len_h << " limited to [" << RECEIVE_BUFF_SIZE << "]\n";
        bdata_len_h = RECEIVE_BUFF_SIZE;
    }
    if(!dev::readall(m_skt, m_buf, bdata_len_h))return -1;
    if(!dev::readall(m_skt, (char*)&frame_end1, 1))return -1;
    if(frame_end1 != frame_end)
    {
        std::cout << "invalid 'frame_end' received [" << (int)frame_end1 << " expected [" << frame_end << "]\n";
        return -1;
    }  
    return bdata_len_h;
};

void dev::ctf_messenger::spin_packets(char* bdata, uint32_t bdata_len)
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_sent = 0;
    size_t print_time = 0;

    while(1)
    {
        if(!send_packet(bdata, bdata_len))return;
        
        if(m_spin_sleep_time_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_spin_sleep_time_ms));
        }

        total_sent += bdata_len;
        ++pnum;
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;
        if(print_time != (size_t)time_delta && time_delta%2 == 0)
        {
            print_time = (size_t)time_delta;
            auto bpsec = int(total_sent / time_delta);
            std::cout << "#" << pnum << " " << dev::size_human(total_sent) << " " << dev::size_human(bpsec) << "/sec" << std::endl;
        }
    }    
};

void dev::ctf_messenger::generate_fix_packets(const dev::fixmsg_view<no_tag_mapper>& fv, const dev::T2G& generators)
{
    std::cout << "starting fix-generator on template[" << fv.strv() << "]\n";
    
    std::string fix_str;
    fix_str.reserve(256);

    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_sent = 0;
    size_t print_time = 0;

    for(const auto& j : generators)
    {
        std::visit([](auto&& g){g.init();}, j.second);
    }
    

    while(1)
    {
        /// generate packet ///
        fix_str.clear();
        const auto& tags = fv.tags();
        for(const auto& i : tags)
        {
            fix_str.append(i.tag_s);
            fix_str.append("=");
        
            auto j = generators.find(i.tag);
            if(j == generators.end())
            {
                fix_str.append(i.value);
            }
            else
            {
                std::visit([&fix_str](auto&& g){fix_str.append(g.next());}, j->second);
            }

            fix_str.append("|");
        }        

        /// send packet ///
        const char* bdata = fix_str.c_str();
        uint32_t bdata_len = fix_str.size();
        if(!send_packet((char*)bdata, bdata_len))return;
        
        if(m_spin_sleep_time_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_spin_sleep_time_ms));
        }

        total_sent += bdata_len;
        ++pnum;
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;
        if(print_time != (size_t)time_delta && time_delta%2 == 0)
        {
            print_time = (size_t)time_delta;
            auto bpsec = int(total_sent / time_delta);
            std::cout << "#" << pnum << " " << dev::size_human(total_sent) << " " << dev::size_human(bpsec) << "/sec" << std::endl;
        }        
    }
};
