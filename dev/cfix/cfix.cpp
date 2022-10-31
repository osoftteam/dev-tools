#include "cfix.h"
#include <unordered_set>
#include <iostream>
#include <cstring>
#include <unordered_map>

using std::cout;
using std::endl;
using std::string;
using std::string_view; 

using COLOR_TAGS = std::unordered_set<int>;
COLOR_TAGS ctags1;
COLOR_TAGS ctags2;
COLOR_TAGS ctags3;

#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLDBLACK   "\033[1m\033[30m"
#define BOLDRED     "\033[1m\033[31m"
#define BOLDGREEN   "\033[1m\033[32m"
#define BOLDYELLOW  "\033[1m\033[33m"
#define BOLDBLUE    "\033[1m\033[34m"
#define BOLDMAGENTA "\033[1m\033[35m"
#define BOLDCYAN    "\033[1m\033[36m"
#define BOLDWHITE   "\033[1m\033[37m"
#define YELLOW_ON_BLUE "\E[33;44m"

static constexpr string_view ctag1_begin = BOLDGREEN;
static constexpr string_view ctag2_begin = BOLDBLUE;
static constexpr string_view ctag3_begin = BOLDMAGENTA;
static constexpr string_view ctag_end    = RESET;

string_view fixtype2name(cfix::fmsg fix)
{
    string_view name = "";
    switch(fix){
    case cfix::fmsg::NewOrd:      name = "[NEW]";break;
    case cfix::fmsg::NewOrdML:    name = "[NEW-ML]";break;
    case cfix::fmsg::ExecAck:     name = "[ACK]";break;
    case cfix::fmsg::ExecPart:    name = "[PART]";break;
    case cfix::fmsg::ExecFill:    name = "[FILL]";break;
    case cfix::fmsg::ExecPartLeg: name = "[PART-LEG]";break;
    case cfix::fmsg::ExecPartML:  name = "[PART-ML]";break;
    case cfix::fmsg::ExecFillLeg: name = "[FILL-LEG]";break;
    case cfix::fmsg::ExecFillML:  name = "[FILL-ML]";break;
    case cfix::fmsg::CancelReq:   name = "[CANCEL-REQ]";break;
    case cfix::fmsg::CancelPending:name = "[CANCEL-PEND]";break;
    case cfix::fmsg::CancelReject: name = "[CANCEL-REJ]";break;
    case cfix::fmsg::Canceled:    name = "[CANCELED]";break;
    case cfix::fmsg::Reject:      name = "[REJECT]";break;
    case cfix::fmsg::RplReqML:    name = "[RPL-ML-REQ]";break;
    case cfix::fmsg::RplReq:      name = "[RPL-REQ]";break;
    case cfix::fmsg::RplPending:  name = "[RPL-PEND]";break;
    case cfix::fmsg::Replaced:    name = "[REPLACED]";break;
    default:name = "";
    }

    return name;
};

string_view get_tag_value(const string_view& sfix, const string_view& _tag)
{
    string tag = string("|") + string(_tag) + "=";
    string rv = "";
    size_t pos_b = sfix.find(tag);
    if(pos_b != string::npos){
        size_t pos_e = sfix.find("|", pos_b + 3);
        if(pos_e !=  string::npos){
            size_t start = pos_b  + tag.length();
            rv = sfix.substr(start, pos_e - start);
        }
    }
    return rv;
};

cfix::fmsg parse_message(const std::string& s, std::string& ord_id, std::string& original_ord_id)
{
    cfix::fix_view fv(s);
    
    ord_id = original_ord_id = "";
    return fv.build_fix_view();
}

string analyse(const string& sfix)
{
    string ord_id = "";
    string original_ord_id = "";
    auto fix = parse_message(sfix, ord_id, original_ord_id);
    auto fix_name= fixtype2name(fix);
    string rv = YELLOW_ON_BLUE  + string(fix_name) + RESET;
    return rv;
};

COLOR_TAGS load_tags(const char* env_variable_name, const char* default_tags)
{
    COLOR_TAGS tags_g;
    FILE *fp;
    char tags_list[256];
    char* tags = nullptr;
 
    string cmd = string("env | grep ") + env_variable_name;
    fp = popen(cmd.c_str(), "r");
    if(fp != nullptr){
        if(fgets(tags_list, sizeof(tags_list), fp) != nullptr){
            tags = strchr(tags_list, '=');
        }
    }
    pclose(fp);

    if(tags == nullptr)
        {
            strncpy(tags_list, default_tags, sizeof(tags_list));
            tags = strchr(tags_list, '=');
        }

    if(tags != nullptr)
        {
            ++tags;
            char* token = strtok(tags, ":");
            while(token != nullptr){
                int tag = atoi(token);
                if(tag != 0){
                    tags_g.insert(tag);
                }
                token = strtok(nullptr, ":");
            }
        }
    return tags_g;
};

void cfix::init_cfix()
{
    ctags1 = load_tags("COLORIZE_FIX_TAGS1", "COLORIZE_FIX_TAGS1=14:32:35:38:39:40:59:150:151:44");
    ctags2 = load_tags("COLORIZE_FIX_TAGS2", "COLORIZE_FIX_TAGS2=442:555:687:192:564:623:624:654:566");
    ctags3 = load_tags("COLORIZE_FIX_TAGS3", "COLORIZE_FIX_TAGS3=17:640:11:12");
};

void apply_cgroup(string& input_s, const string_view& ctag_begin, const COLOR_TAGS& tags)
{
    for(const auto& t :  tags){
        static char fbegin[32];
        sprintf(fbegin, "|%d=", t);
        size_t search_start = 0;

        while(search_start != string::npos)
        {
            bool colorize = false;
            size_t pos = input_s.find(fbegin, search_start);
            search_start = string::npos;
            if(pos != string::npos)
            {
                input_s.insert(pos + 1, ctag_begin);
                colorize = true;
            }

            if(colorize){
                pos = input_s.find("|", pos + 3);
                if(pos != string::npos){
                    input_s.insert(pos, ctag_end);
                    search_start = pos + ctag_end.length();
                }
            }
        }
    }
}

void cfix::colorize_stdin()
{
    char buff[4096];
    memset(buff, 0, sizeof(buff));
    while(fgets(buff, sizeof(buff), stdin))
    {
        string input_s(buff);
        string fix_type = analyse(input_s);
        apply_cgroup(input_s, ctag1_begin, ctags1);
        apply_cgroup(input_s, ctag2_begin, ctags2);
        apply_cgroup(input_s, ctag3_begin, ctags3);
        cout << fix_type << input_s;
    }
};

void cfix::run_test()
{
    char buff[] = "[23/03/2012 12:51:06.057245]8=FIX.4.4|9=334|35=AB|49=CLIHUB|56=FLINKI_PJ|115=CLIENT_PJ|116=CS1|50=A1|34=6|52=20120323-12:51:06.053|11=CAA0001|38=2000000|55=AUD/USD|15=AUD|40=D|117=0b4b91a825|54=1|167=NONE|554=HWfrfnUAKa|555=2|654=CAA0001_1|600=AUD/USD|588=20120328|687=1000000|624=2|566=1.057515|564=O|654=CAA0001_2|600=AUD/USD|588=20120405|687=1000000|624=1|566=1.057477|564=C|10=000|";

    cout << "cfix - running test" << endl;
    string input_s(buff);
    string fix_type = analyse(input_s);
    apply_cgroup(input_s, ctag1_begin, ctags1);
    apply_cgroup(input_s, ctag2_begin, ctags2);
    apply_cgroup(input_s, ctag2_begin, ctags3);
    cout << fix_type << input_s;
};

using FTAGS = std::unordered_map<int, std::string_view>;
void print_tags(const FTAGS& ftags)
{
    for(const auto& t : ftags)
    {
        std::cout << "(" << t.first << "," << t.second << ")" << "\n";
    }
}

cfix::fix_view::fix_view(const std::string_view& fix)noexcept:m_fix(fix)
{

};


cfix::fmsg cfix::fix_view::build_fix_view()
{
    m_message_type = cfix::fmsg::Uknown;
    
    FTAGS ftags;
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

        //std::cout << "(" << tag << "," << val << ")" << "\n";

        auto tag_num = atoi(tag.data());
        switch(tag_num)
        {
        case 35:m_tag35 = val;break;
        case 39:m_tag39 = val;break;
        case 442:m_tag442 = val;break;
        default:break;
        }
        ftags.emplace(tag_num, val); 

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
    
    print_tags(ftags);
    return m_message_type;
};

void cfix::fix_view::parse_message_type()
{
    m_message_type = cfix::fmsg::Uknown;
    auto size35 = m_tag35.size();
    switch(size35)
    {
    case 0:return;
    case 1:
    {
        switch(m_tag35[0])
        {
        case 'D':m_message_type = cfix::fmsg::NewOrd;break;
        case 'F':m_message_type = cfix::fmsg::CancelReq;break;
        case 'G':m_message_type = cfix::fmsg::RplReq;break;
        case '9':m_message_type = cfix::fmsg::CancelReject;break;
        case '3':m_message_type = cfix::fmsg::Reject;break;
        case '8':
        {
            auto size39 = m_tag39.size();
            switch(size39)
            {
            case 0:return;
            case 1:
            {
                case '0':m_message_type = cfix::fmsg::ExecAck;break;
                case '6':m_message_type = cfix::fmsg::CancelPending;break;
                case '4':m_message_type = cfix::fmsg::Canceled;break;
                case '8':m_message_type = cfix::fmsg::Reject;break;
                case 'E':m_message_type = cfix::fmsg::RplPending;break;
                case '5':m_message_type = cfix::fmsg::Replaced;break;
                case '1':
                {
                    m_message_type = cfix::fmsg::ExecPart;
                    auto size442 = m_tag442.size();
                    switch(size442)
                    {
                    case 1:
                    {
                        switch(m_tag442[0])
                        {
                        case '2':m_message_type = cfix::fmsg::ExecPartLeg;break;
                        case '3':m_message_type = cfix::fmsg::ExecPartML;break;
                        }
                    }break;
                    }
                }break;
                case '2':
                {
                    m_message_type = cfix::fmsg::ExecPart;
                    auto size442 = m_tag442.size();
                    switch(size442)
                    {
                    case 1:
                    {
                        switch(m_tag442[0])
                        {
                        case '2':m_message_type = cfix::fmsg::ExecFillLeg;break;
                        case '3':m_message_type = cfix::fmsg::ExecFillML;break;
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
        case 'B':m_message_type = cfix::fmsg::NewOrdML;break;
        case 'C':m_message_type = cfix::fmsg::RplReqML;break;
        default:return;
        }        
    }break;///2bytes in 35
    }    
};
