#pragma once
#include <variant>
#include <iostream>
#include "dev-utils.h"

namespace dev
{
    class set_tag_stat
    {
    public:
        void update_stat(const std::string_view& s);
        std::string to_string()const;
        
    private:
        dev::S2N m_stat;
        friend std::ostream& operator<<(std::ostream& os, const dev::set_tag_stat& t);
    };
    
    class range_tag_stat
    {
    public:
        void update_stat(const std::string_view& s);
        std::string to_string()const;

    private:
        size_t m_range_begin{}, m_range_end{};
        friend std::ostream& operator<<(std::ostream& os, const dev::range_tag_stat& t);
    };        
    using var_tag_stat = std::variant<set_tag_stat, range_tag_stat>;


    
    template<class S>
    class tag_generator
    {
    public:
        var_tag_stat make_tag_stat()const;
    };

    class set_tag_generator:public tag_generator<set_tag_stat>
    {
        // (AAPL, MSFT, IBM)
    private:
        STRINGS m_data;
        mutable STRINGS::const_iterator m_it;
    public:
        void init()const;
        std::string next()const;
        std::string to_string()const;
        
        friend class tag_generator_factory;
        friend std::ostream& operator<<(std::ostream& os, const dev::set_tag_generator& t);
    };
    
    class range_tag_generator:public tag_generator<range_tag_stat>
    {
        // [100..1000,200]
    private:
        size_t m_range_begin{}, m_range_end{}, m_range_step{};
        mutable size_t m_value{};
    public:
        void init()const;
        std::string next()const;
        std::string to_string()const;
        
        friend class tag_generator_factory;
        friend std::ostream& operator<<(std::ostream& os, const dev::range_tag_generator& t);
    };


    using var_generator = std::variant<set_tag_generator, range_tag_generator>;
    using T2G           = std::unordered_map<size_t, var_generator>;
    using T2STAT        = std::unordered_map<size_t, var_tag_stat>;
    
    class tag_generator_factory
    {
    public:
        static std::optional<var_generator> produce_generator(const std::string_view& s);
    };

    class stat_tag_mapper
    {
        dev::T2STAT& m_stat;
    public:
        stat_tag_mapper(dev::T2STAT& stat):m_stat(stat){}
    
        bool map_tag(size_t tag, const std::string_view& sv);
    };
    

    std::ostream& operator<<(std::ostream& os, const dev::set_tag_stat& t);
    std::ostream& operator<<(std::ostream& os, const dev::range_tag_stat& t);
    std::ostream& operator<<(std::ostream& os, const dev::set_tag_generator& t);
    std::ostream& operator<<(std::ostream& os, const dev::range_tag_generator& t);
}

template<class S>
dev::var_tag_stat dev::tag_generator<S>::make_tag_stat()const
{
    S st;
    dev::var_tag_stat rv{st};
    return rv;
};
