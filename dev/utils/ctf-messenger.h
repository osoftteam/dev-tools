#pragma once
#include "dev-utils.h"
#include "skt-utils.h"
#include "fix-utils.h"
#include "tag-generator.h"

namespace dev
{
    
    template<class SK = dev::blocking_tcp_socket>
    class ctf_messenger
    {
    public:
        bool init(int skt, size_t spin_sleep_time_ms);
        void spin_packets(const ctf_packet& pkt, uint32_t wire_len);
        void generate_fix_packets(const dev::fixmsg_view<no_tag_mapper>& fv, const dev::T2G& generators);
        void receive_packets();
        void receive_fix_messages(T2STAT& stat);
        SK&  sk(){return m_sk;}
    private:
        bool send_packet(const ctf_packet& pkt, uint32_t wire_len);
        int  read_packet();

        SK         m_sk;
        size_t     m_spin_sleep_time_ms{0};
        ctf_packet m_pkt;
    };
};


extern bool print_all_data;
extern bool collect_statistics;
extern size_t max_num_packets;


template<class SK>
bool dev::ctf_messenger<SK>::init(int skt, size_t sleep_time)
{
    m_sk.init(skt);
    m_spin_sleep_time_ms = sleep_time;
    return true;
};

template<class SK>
bool dev::ctf_messenger<SK>::send_packet(const ctf_packet& pkt, uint32_t wire_len)
{
    if(!m_sk.sendall((char*)&pkt, wire_len))return false;
    return true;
};

template<class SK>
int dev::ctf_messenger<SK>::read_packet()
{
    return m_sk.read_packet(m_pkt);
};


template<class SK>
void dev::ctf_messenger<SK>::receive_packets()
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_received = 0;
    size_t print_time = 0;
    
    while(1)
    {
        auto bdata_len = read_packet();
        if(bdata_len < 0)return;
        if(bdata_len == 0)continue;
        total_received += bdata_len;
        ++pnum;

        if(print_all_data){
            m_pkt.data[bdata_len] = 0;
            std::cout << m_pkt.data << std::endl;
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

template<class SK>
void dev::ctf_messenger<SK>::receive_fix_messages(T2STAT& stat)
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
        if(bdata_len < 0)return;
        if(bdata_len == 0)continue;
        std::string_view s(m_pkt.data, bdata_len);
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

template<class SK>
void dev::ctf_messenger<SK>::spin_packets(const ctf_packet& pkt, uint32_t wire_len)
{
    auto session_start = time(nullptr);
    size_t pnum = 0;
    size_t total_sent = 0;
    size_t print_time = 0;

    while(1)
    {
        if(!send_packet(pkt, wire_len))return;
        
        if(m_spin_sleep_time_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_spin_sleep_time_ms));
        }

        total_sent += wire_len;
        ++pnum;
        
        auto now = time(nullptr);
        auto time_delta = now - session_start;
        if(print_time != (size_t)time_delta && time_delta%2 == 0)
        {
            print_time = (size_t)time_delta;
            auto bpsec = int(total_sent / time_delta);
            std::cout << "#" << pnum << " " << dev::size_human(total_sent) << " " << dev::size_human(bpsec) << "/sec" << std::endl;
        }

        if(max_num_packets != 0 && pnum > max_num_packets)
        {
            std::cout << "reached max number of packets" << std::endl;
            return;
        }
    }    
};

template<class SK>
void dev::ctf_messenger<SK>::generate_fix_packets(const dev::fixmsg_view<no_tag_mapper>& fv, const dev::T2G& generators)
{
    std::cout << "starting fix-generator on template[" << fv.strv() << "]\n";

    std::string fix_str;
    fix_str.reserve(512);

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
        uint32_t bdata_len = fix_str.size();
        auto data = fix_str.data();
        if(bdata_len > CTF_MAX_PAYLOAD)
        {
            bdata_len = CTF_MAX_PAYLOAD;
            std::cout << "fix payload oversize [" << bdata_len << "] \n";
        }
        for(uint32_t i = 0; i < bdata_len; ++i){
            m_pkt.data[i] = data[i];
        }
        uint32_t wire_len = m_pkt.setup_packet(bdata_len);
        if(!send_packet(m_pkt, wire_len))return;
        
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
