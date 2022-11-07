#pragma once
#include <variant>
#include "dev-utils.h"

namespace dev
{    
    class data_set_tag_generator
    {
        /// (AAPL, MSFT, IBM) ///
    private:
        STRINGS m_data;
        mutable STRINGS::const_iterator m_it;
    public:
        void init()const;
        std::string next()const;
        std::string rule_to_string()const;
        friend class tag_generator_factory;
    };
    
    class dense_range_tag_generator
    {
        /// [100..1000,200] ///
    private:
        size_t m_range_begin{}, m_range_end{}, m_range_step{};
        mutable size_t m_value{};
    public:
        void init()const;
        std::string next()const;
        std::string rule_to_string()const;
        friend class tag_generator_factory;
    };

    using var_generator = std::variant<data_set_tag_generator, dense_range_tag_generator>;

    class tag_generator_factory
    {
    public:
        static std::optional<var_generator> produce_generator(const std::string_view& s);
    };

    class tag_generator_stringer
    {
    public:
        void operator()(const data_set_tag_generator& g)const{m_rule_str = g.rule_to_string();}
        void operator()(const dense_range_tag_generator& g)const{m_rule_str = g.rule_to_string();}
        std::string str()const{return m_rule_str;}
    private:
        mutable std::string m_rule_str;
    };

    class tag_generator_value
    {
    public:
        void operator()(const data_set_tag_generator& g)const{m_value = g.next();}
        void operator()(const dense_range_tag_generator& g)const{m_value = g.next();}
        std::string value()const{return m_value;}
    private:
        mutable std::string m_value;
    };

    class tag_generator_init
    {
    public:
        void operator()(const data_set_tag_generator& g)const{g.init();}
        void operator()(const dense_range_tag_generator& g)const{g.init();}
    };

    
    
    using T2G = std::unordered_map<size_t, var_generator>;
}
