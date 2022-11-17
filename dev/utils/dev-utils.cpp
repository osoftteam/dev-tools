#include <sys/socket.h>
#include <unistd.h>
#include "dev-utils.h"


size_t dev::stoui(const std::string_view& s)
{
    static size_t arr10[] = {10,100,1000,10000,100000,1000000, 10000000}; 
    if(s.empty())return 0;
    auto i = s.rbegin();

    char digit = *i;
    if(!std::isdigit(digit))return 0;
    ++i;
    size_t rv = (digit - '0');
    size_t arr_idx = 0;
    while(i != s.rend())
    {
        if(arr_idx > sizeof(arr10)){
            std::cerr << "stoui - out of digits" << std::endl;
            return 0;
        }
        digit = *i;
        if(!std::isdigit(digit))return 0;
        rv += (digit - '0') * arr10[arr_idx];
        ++i;
        ++arr_idx;
    }
    return rv;
}

std::optional<dev::cfg_info> dev::read_config(const std::string& file_name)
{    
    std::optional<dev::cfg_info>rv = {};    
    std::string s;
    std::ifstream f (file_name);
    if (f.is_open())
    {
        cfg_info cfg;
        while ( getline (f, s) )
        {
            s = dev::trim(s);
            auto p = s.find('=');
            if(p != std::string::npos){
                auto prop = s.substr(0, p);
                prop = dev::trim(prop);
                auto val = s.substr(p+1);
                val = dev::trim(val);
                if(!prop.empty() && !val.empty())
                {
                    if(prop[0] != '#')
                    {
                        p = prop.find("tag.");
                        if(p == 0)
                        {
                            auto t = dev::stoui(prop.substr(4));
                            if(t != 0)
                            {
                                cfg.tags[t] = val;
//                                std::cout << "got-tag:" << prop << " @" << p << " t=" << t << std::endl;
                            }
                        }
                        else
                        {
                            cfg.params[prop] = val;
                        }
                    }
                }
            }
        }
        rv.emplace(std::move(cfg));
        f.close();
    }    
    else
    {
        return rv;
    }
    
    return rv;
}

namespace dev
{
    template<class T = std::string>
    T& ltrim(T &s)
    {
        auto it = std::find_if(s.begin(), s.end(),
                               [](char c) {
                                   return !std::isspace<char>(c, std::locale::classic());
                               });
        s.erase(s.begin(), it);
        return s;
    }

    template<class T = std::string>
    T& rtrim(T &s)
    {
        auto it = std::find_if(s.rbegin(), s.rend(),
                               [](char c) {
                                   return !std::isspace<char>(c, std::locale::classic());
                               });
        s.erase(it.base(), s.end());
        return s;
    }
}

std::string& dev::trim(std::string &s) {
    return ltrim(rtrim(s));
}

std::string dev::size_human(uint64_t bytes, bool bytes_units)
{
    static constexpr char len = 5;
	static constexpr char suffix_bytes[len][3] = {"B", "KB", "MB", "GB", "TB"};
    static constexpr char suffix[len][3] = {"", "K", "M", "G", "T"};

	int i = 0;
	double dblBytes = bytes;

	if (bytes > 1024)
    {
		for (i = 0; (bytes / 1024) > 0 && i<len; i++, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}

    char output[64];
    if(bytes_units){
        sprintf(output, "%.02lf%s", dblBytes, suffix_bytes[i]);
    }
    else{
        sprintf(output, "%.02lf%s", dblBytes, suffix[i]);
    }
    std::string rv{output};
    return output;
}

