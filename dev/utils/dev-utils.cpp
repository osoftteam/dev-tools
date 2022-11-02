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
    int arr_idx = 0;
    while(i != s.rend())
    {
        digit = *i;
        if(!std::isdigit(digit))return 0;
        rv += (digit - '0') * arr10[arr_idx];
        ++i;
        ++arr_idx;
    }
    return rv;
}

std::optional<dev::S2S> dev::read_config(const std::string& file_name)
{    
    std::optional<S2S>rv = {};    
    std::string s;
    std::ifstream f (file_name);
    if (f.is_open())
    {
        S2S m;
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
                    if(prop[0] != '#'){
                        m[prop] = val;
                    }
                }
            }
        }
        rv.emplace(std::move(m));
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

std::string dev::size_human(uint64_t bytes)
{
    static constexpr char len = 5;
	static constexpr char suffix[len][3] = {"B", "KB", "MB", "GB", "TB"};

	int i = 0;
	double dblBytes = bytes;

	if (bytes > 1024)
    {
		for (i = 0; (bytes / 1024) > 0 && i<len; i++, bytes /= 1024)
			dblBytes = bytes / 1024.0;
	}

    char output[64];
    sprintf(output, "%.02lf%s", dblBytes, suffix[i]);
    std::string rv{output};
    return output;
}

bool dev::sendall(int s, char *buf, size_t len)
{    
    size_t total = 0;
    int bytesleft = len;
    int n = 0;
    while(total < len)
    {
       n = send(s, buf+total, bytesleft, 0);
       if (n == -1) {
           perror("socket-send");
           return false;
       }
       total += n;
       bytesleft -= n;
    }
    return true;
};

bool dev::readall(int s, char *buf, size_t len)
{
    size_t total = 0;
    int bytesleft = len;
    int n = 0;
    while(total < len)
    {
        n = read(s, buf+total, bytesleft);
        if (n == -1) {
            perror("socket-read");
            return false;
        }
        total += n;
        bytesleft -= n;
    }
    return true;
};
