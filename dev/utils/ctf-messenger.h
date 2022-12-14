#pragma once
#include "dev-utils.h"
#include "skt-utils.h"
#include "fix-utils.h"
#include "tag-generator.h"

namespace dev
{
extern bool parse_fix;
extern bool collect_statistics;
extern bool print_all_data;
    
    template<class SK = dev::blocking_tcp_socket>
    class ctf_messenger
    {
    public:
        bool init(int skt, size_t spin_sleep_time_ms);
        void spin_packets(const ctf_packet& pkt, datalen_t wire_len);
        void generate_fix_packets(const dev::fixmsg_view<no_tag_mapper>& fv, const dev::T2G& generators);
        void receive_packets();
        SK&  sk(){return m_sk;}
    private:
        SK         m_sk;
        size_t     m_spin_sleep_time_ms{0};
    };
};


template<class SK>
bool dev::ctf_messenger<SK>::init(int skt, size_t sleep_time)
{
    m_sk.init(skt);
    m_spin_sleep_time_ms = sleep_time;
    return true;
};

template<class SK>
void dev::ctf_messenger<SK>::receive_packets()
{
    std::cout << "receiving packets.. \n";  
    m_sk.start_session();
    while(1)
    {        
        auto r = m_sk.read_packet();
        if(r.len < 0)return;
        if(r.len == 0)continue;

        
        if(print_all_data){
            std::cout << m_sk.pkt().data << std::endl;
        }
    }
};

template<class SK>
void dev::ctf_messenger<SK>::spin_packets(const ctf_packet& pkt, datalen_t wire_len)
{
    auto session_start = time(nullptr);
    uint32_t pnum = 0;
    size_t total_sent = 0;
    size_t print_time = 0;

    while(1)
    {
        pkt.setseq(pnum);
        if(!m_sk.send_packet(pkt, wire_len))return;

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
    }
};

template<class SK>
void dev::ctf_messenger<SK>::generate_fix_packets(const dev::fixmsg_view<no_tag_mapper>& fv, const dev::T2G& generators/*, size_t pkt_counter_tag*/)
{
    std::cout << "starting fix-generator on template[" << fv.strv() << "]\n";

    ctf_packet m_pkt;
    
    std::string fix_str;
    fix_str.reserve(512);

    auto session_start = time(nullptr);
    uint32_t pnum = 0;
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
        datalen_t bdata_len = fix_str.size();
        auto data = fix_str.data();
        if(bdata_len > CTF_MAX_PAYLOAD)
        {
            bdata_len = CTF_MAX_PAYLOAD;
            std::cout << "fix payload oversize [" << bdata_len << "] \n";
        }
        for(datalen_t i = 0; i < bdata_len; ++i){
            m_pkt.data[i] = data[i];
        }
        auto wire_len = m_pkt.setup_packet(bdata_len);
        m_pkt.setseq(pnum);        
        if(!m_sk.send_packet(m_pkt, wire_len))return;
        
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
