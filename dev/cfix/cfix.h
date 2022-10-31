#pragma once

#include <string_view>

namespace cfix
{
    enum class fmsg{
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

    void init_cfix();
    void colorize_stdin();
    void run_test();

    class fix_view
    {
    public:
        fix_view(const std::string_view& fix)noexcept;
        fmsg build_fix_view();
    private:
        void parse_message_type();
        
        const std::string_view& m_fix;
        fmsg             m_message_type{cfix::fmsg::Uknown};
        std::string_view m_tag35;
        std::string_view m_tag39;
        std::string_view m_tag442;
    };
};
