#pragma once

#include "dev-utils.h"

namespace dev
{
    enum class fixmsg{
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
        std::string_view value;
    };
    using FIXTAGS = std::vector<fixtag_value>;

    
    class fixmsg_view
    {
    public:
        fixmsg_view(const std::string_view& fix)noexcept;
        fixmsg build_fix_view();
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
