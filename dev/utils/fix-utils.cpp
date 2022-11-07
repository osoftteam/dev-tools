#include "fix-utils.h"


std::string_view dev::fixtype2name(dev::fixmsg fix)
{
    std::string_view name = "";
    switch(fix){
    case dev::fixmsg::NewOrd:      name = "[NEW]";break;
    case dev::fixmsg::NewOrdML:    name = "[NEW-ML]";break;
    case dev::fixmsg::ExecAck:     name = "[ACK]";break;
    case dev::fixmsg::ExecPart:    name = "[PART]";break;
    case dev::fixmsg::ExecFill:    name = "[FILL]";break;
    case dev::fixmsg::ExecPartLeg: name = "[PART-LEG]";break;
    case dev::fixmsg::ExecPartML:  name = "[PART-ML]";break;
    case dev::fixmsg::ExecFillLeg: name = "[FILL-LEG]";break;
    case dev::fixmsg::ExecFillML:  name = "[FILL-ML]";break;
    case dev::fixmsg::CancelReq:   name = "[CANCEL-REQ]";break;
    case dev::fixmsg::CancelPending:name = "[CANCEL-PEND]";break;
    case dev::fixmsg::CancelReject: name = "[CANCEL-REJ]";break;
    case dev::fixmsg::Canceled:    name = "[CANCELED]";break;
    case dev::fixmsg::Reject:      name = "[REJECT]";break;
    case dev::fixmsg::RplReqML:    name = "[RPL-ML-REQ]";break;
    case dev::fixmsg::RplReq:      name = "[RPL-REQ]";break;
    case dev::fixmsg::RplPending:  name = "[RPL-PEND]";break;
    case dev::fixmsg::Replaced:    name = "[REPLACED]";break;
    default:name = "";
    }

    return name;
};

const dev::S2S& dev::fixmsg_sample()
{
    static dev::S2S ftags = {
        {"AB", "8=FIX.4.4|9=334|35=AB|49=CLIHUB|56=FLINKI_PJ|115=CLIENT_PJ|116=CS1|50=A1|34=6|52=20120323-12:51:06.053|11=CAA0001|38=2000000|55=AUD/USD|15=AUD|40=D|117=0b4b91a825|54=1|167=NONE|554=HWfrfnUAKa|555=2|654=CAA0001_1|600=AUD/USD|588=20120328|687=1000000|624=2|566=1.057515|564=O|654=CAA0001_2|600=AUD/USD|588=20120405|687=1000000|624=1|566=1.057477|564=C|10=000|"},
        {"D", "8=FIX.4.4|9=473|35=D|34=5|49=SENDER2|52=20230122-20:56:57|56=TARGET3|11=80217_94596|22=5|48=AAPL|15=USD|21=3|38=111|40=1|54=2|55=AAPL|59=0|64=20221201|461=ABCD|77=O|202=145.0|207=OPRA|200=202301|120=USD|60=20230122-19:21:41.919|453=2|448=abcd|447=C|452=11|448=MyBank|447=B|452=29|541=20230120|228=1.00000000|6499=N|561=1.0|562=1.0|7647=TEST1-OVER-TEST2|957=2|958=BROKER_REASON|959=14|960=Execution Negotiated|958=PM|959=14|960=abcd|10=006|"}
    };

    return ftags;
};
