#pragma once
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <optional>

namespace dev
{
    using S2S = std::unordered_map<std::string, std::string>;
    using SSET = std::unordered_set<std::string>;
    using STRINGS = std::vector<std::string>;

    size_t stoui(const std::string_view& s);
    std::optional<dev::S2S> read_config(const std::string& file_name);    
    std::string& trim(std::string &s);    
    std::string size_human(uint64_t bytes);
    
    template<class IT>
    void print_pairs(IT b, IT e, const std::string sep = " ");

    bool sendall(int s, char *buf, size_t len);
    bool readall(int s, char *buf, size_t len);
};


template<class IT> void dev::print_pairs(IT b, IT e, const std::string sep)
{
    auto i = b;
    while(i != e)
    {
        std::cout << i->first << sep << i->second << "\n";
        ++i;
    }
};
