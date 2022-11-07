#include <iostream>
#include <cstring>
#include "dev-utils.h"
#include "fix-utils.h"
#include "pfix.h"

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
bool paint_fix_tags = true;

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
            strncpy(tags_list, default_tags, sizeof(tags_list)-1);
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
    if(paint_fix_tags)
    {
        FILE *fp;
        char tags_list[256];
        char* tags = nullptr;
 
        string cmd = string("env | grep COLORIZE_FIX_TAGS1");
        fp = popen(cmd.c_str(), "r");
        if(fp != nullptr){
            if(fgets(tags_list, sizeof(tags_list), fp) != nullptr){
                tags = strchr(tags_list, '=');
            }
        }
        if(tags != nullptr && tags[0] != 0)
        {
            ++tags;

            std::string val(tags);
            val = dev::trim(val);
            if(val == "none")
            {
                paint_fix_tags = false;
            }
        }
    
        if(paint_fix_tags)
        {
            ctags1 = load_tags("COLORIZE_FIX_TAGS1", "COLORIZE_FIX_TAGS1=14:32:35:38:39:40:59:150:151:44");
            ctags2 = load_tags("COLORIZE_FIX_TAGS2", "COLORIZE_FIX_TAGS2=442:555:687:192:564:623:624:654:566");
            ctags3 = load_tags("COLORIZE_FIX_TAGS3", "COLORIZE_FIX_TAGS3=17:640:11:12");
        }
    }
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

static void print_fix_string_in_color(std::string& s)
{
    dev::fixmsg_view fv(s);
    auto fix = fv.message_type();
    auto fix_name = dev::fixtype2name(fix);
    if(paint_fix_tags)
    {
        apply_cgroup(s, ctag1_begin, ctags1);
        apply_cgroup(s, ctag2_begin, ctags2);
        apply_cgroup(s, ctag2_begin, ctags3);
        cout << YELLOW_ON_BLUE;
    }    
    cout << fix_name; 
    if(paint_fix_tags)
    {   
        cout << RESET;
    }
    cout << s;
    cout << std::endl;
    fv.print_tags();
    cout << std::endl;
};

void cfix::colorize_stdin()
{
    char buff[4096];
    memset(buff, 0, sizeof(buff));
    while(fgets(buff, sizeof(buff), stdin))
    {
        string s(buff);
        print_fix_string_in_color(s);
    }
};

void cfix::run_test()
{
    auto& lst = dev::fixmsg_sample();       
    for(auto& fx : lst)
    {
        auto s = fx.second;
        print_fix_string_in_color(s);
    }
};

