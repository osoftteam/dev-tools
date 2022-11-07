#pragma once

#include "dev-utils.h"

namespace dev
{
    enum class fixmsg{
        Ignored = -2,
        Uknown = -1,
        NewOrd,
        NewOrdML,
        ExecAck,
        ExecPart,
        ExecFill,
        ExecPartLeg,
        ExecPartML,
        ExecFillLeg,
        ExecFillML,
        CancelReq,
        CancelPending,
        Canceled,
        CancelReject,
        Reject,
        RplReqML,
        RplReq,
        RplPending,
        Replaced
    };

    std::string_view fixtype2name(dev::fixmsg fix);

    struct fixtag_value
    {
        size_t           tag;
        std::string_view tag_s;
        std::string_view value;
    };
    using FIXTAGS = std::vector<fixtag_value>;

    class no_tag_mapper
    {
    public:
        bool map_tag(size_t, const std::string_view&){return true;};
    };

    template<class TM=no_tag_mapper>
    class fixmsg_view
    {
    public:
        fixmsg_view(const std::string_view& fix, TM* tm = nullptr);
        const std::string_view& strv()const{return m_fix;}
        fixmsg message_type()const{return m_message_type;}
        const FIXTAGS& tags()const{return m_tags;}
        void print_tags()const;
        
    private:
        void parse_message_type();
        
        const std::string_view& m_fix;
        fixmsg           m_message_type{dev::fixmsg::Uknown};
        std::string_view m_tag35;
        std::string_view m_tag39;
        std::string_view m_tag442;
        FIXTAGS          m_tags;
    };

    /// example of fix tags grouped by type (35)
    const S2S& fixmsg_sample();
}


template<class TM>
dev::fixmsg_view<TM>::fixmsg_view(const std::string_view& fix, TM* tm):m_fix(fix)
{
    m_tags.reserve(32);
    
    auto p = std::begin(m_fix);
    auto e = std::end(m_fix); 

    auto ptag = p;
    auto pval = p;
    while(p != e)
    {
        while(p != e && *p != '=') ++p;
        if(p == e)break;
        std::string_view tag(ptag, p-ptag);
        ++p;
        pval = p;
        while(p != e && *p != '|') ++p;

        std::string_view val(pval, p-pval);

        auto tag_num = dev::stoui(tag);
        switch(tag_num)
        {
        case 35:m_tag35 = val;break;
        case 39:m_tag39 = val;break;
        case 442:m_tag442 = val;break;
        default:break;
        }
        if(tm)
        {
            if(!tm->map_tag(tag_num, val)){
                m_message_type = dev::fixmsg::Ignored;
                return;
            }
        }
        
        m_tags.emplace_back(fixtag_value{tag_num, tag, val});

        if(p == e)break;
        ++p;
        ptag = p;
        if(p == e)break;
        ++p;
    }

    if(!m_tag35.empty())
    {
        parse_message_type();
    }
};

template<class TM>
void dev::fixmsg_view<TM>::parse_message_type()
{
    m_message_type = dev::fixmsg::Uknown;
    auto size35 = m_tag35.size();
    switch(size35)
    {
    case 0:return;
    case 1:
    {
        switch(m_tag35[0])
        {
        case 'D':m_message_type = dev::fixmsg::NewOrd;break;
        case 'F':m_message_type = dev::fixmsg::CancelReq;break;
        case 'G':m_message_type = dev::fixmsg::RplReq;break;
        case '9':m_message_type = dev::fixmsg::CancelReject;break;
        case '3':m_message_type = dev::fixmsg::Reject;break;
        case '8':
        {
            auto size39 = m_tag39.size();
            switch(size39)
            {
            case 0:return;
            case 1:
            {
                case '0':m_message_type = dev::fixmsg::ExecAck;break;
                case '6':m_message_type = dev::fixmsg::CancelPending;break;
                case '4':m_message_type = dev::fixmsg::Canceled;break;
                case '8':m_message_type = dev::fixmsg::Reject;break;
                case 'E':m_message_type = dev::fixmsg::RplPending;break;
                case '5':m_message_type = dev::fixmsg::Replaced;break;
                case '1':
                {
                    m_message_type = dev::fixmsg::ExecPart;
                    auto size442 = m_tag442.size();
                    switch(size442)
                    {
                    case 1:
                    {
                        switch(m_tag442[0])
                        {
                        case '2':m_message_type = dev::fixmsg::ExecPartLeg;break;
                        case '3':m_message_type = dev::fixmsg::ExecPartML;break;
                        }
                    }break;
                    }
                }break;
                case '2':
                {
                    m_message_type = dev::fixmsg::ExecPart;
                    auto size442 = m_tag442.size();
                    switch(size442)
                    {
                    case 1:
                    {
                        switch(m_tag442[0])
                        {
                        case '2':m_message_type = dev::fixmsg::ExecFillLeg;break;
                        case '3':m_message_type = dev::fixmsg::ExecFillML;break;
                        }
                    }break;
                    }
                }break;                
            }break;
            default:return;
            };
        }break;///8
        }
    }break;///1byte in 35
    case 2:
    {
        switch(m_tag35[1])
        {
        case 'B':m_message_type = dev::fixmsg::NewOrdML;break;
        case 'C':m_message_type = dev::fixmsg::RplReqML;break;
        default:return;
        }        
    }break;///2bytes in 35
    }    
};


template<class TM>
void dev::fixmsg_view<TM>::print_tags()const
{
    for(const auto& i : m_tags){
        std::cout << "(" << i.tag << "," << i.value << ")";
    }
};
