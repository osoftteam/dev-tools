#pragma once
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <optional>

namespace dev
{
    using S2S = std::unordered_map<std::string, std::string>;
    
    template<class T = size_t>T stoui(const std::string_view& s);
    template<class T = std::string> T& trim(T &s);

    template<class M = S2S> std::optional<M> read_config(const std::string& file_name);
    template<class IT> void print_pairs(IT b, IT e, const std::string sep = " ");
    template<class T = std::string> std::string size_human(uint64_t bytes);
};


template<class T = size_t>
T dev::stoui(const std::string_view& s)
{
    static size_t arr10[] = {10,100,1000,10000,100000,1000000, 10000000}; 
    if(s.empty())return 0;
    auto i = s.rbegin();

    char digit = *i;
    if(!std::isdigit(digit))return 0;
    ++i;
    T rv = (digit - '0');
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

template<class T = size_t>
T& dev::trim(T &s) {
    return ltrim(rtrim(s));
}

template<class M = dev::S2S> 
std::optional<M> dev::read_config(const std::string& file_name)
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


template<class IT> void dev::print_pairs(IT b, IT e, const std::string sep)
{
    auto i = b;
    while(i != e)
    {
        std::cout << i->first << sep << i->second << "\n";
        ++i;
    }
};

template<class T> std::string dev::size_human(uint64_t bytes)
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
    T rv{output};
    return output;
}
