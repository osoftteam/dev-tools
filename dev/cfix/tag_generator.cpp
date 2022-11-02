#include "tag_generator.h"

std::string dev::data_set_tag_generator::next()const
{
    if(m_data.empty())return "";
    if(m_it == m_data.end())m_it = m_data.cbegin();
    auto rv = *m_it;
    ++m_it;
    return rv;
};

std::string dev::data_set_tag_generator::rule_to_string()const
{
    std::string rv = "(";
    if(!m_data.empty())
    {
        for(const auto& s : m_data){
            rv += s;
            rv += ",";
        }
        rv.erase(rv.size()-1, 1);
    }
    rv += ")";
    return rv;
};

std::string dev::dense_range_tag_generator::next()const
{
    if(m_value >= m_range_end)m_value = m_range_begin;
    m_value += m_range_step;
    return std::to_string(m_value);
};

std::string dev::dense_range_tag_generator::rule_to_string()const
{
    std::string rv = "[";
    rv += std::to_string(m_range_begin);
    rv += "..";
    rv += std::to_string(m_range_end);
    rv += ",";
    rv += std::to_string(m_range_step);
    rv += "]";
    return rv;
};

std::optional<dev::var_generator> dev::tag_generator_factory::produce_generator(const std::string_view& s)
{
    if(s.size() < 3)return {};
    if(s[0] == '(')
    {
        data_set_tag_generator g;
        
        size_t idx_b = 1;
        auto p = s.find_first_of(",)");
        while(p != std::string::npos){
            std::string s1 = std::string(s.substr(idx_b, p-idx_b));
            s1 = dev::trim(s1);
            if(!s1.empty())
            {
                g.m_data.insert(s1);
            }
            idx_b = p + 1;
            //std::cout << p << " " << s1 << std::endl;
            p = s.find_first_of(",)", idx_b);
        }
        g.m_it = g.m_data.begin();
        return dev::var_generator{g};
    }
    else if(s[0] == '[')
    {
        dense_range_tag_generator g;
        
        std::cout << "parsing= " << s << std::endl;
        size_t idx_b = 1;
        auto r_dots = s.find("..", 1);
        if(r_dots == std::string::npos)return {};
        std::string_view range_begin = s.substr(idx_b, r_dots-idx_b);
        std::cout << "range_begin= " << range_begin << std::endl;
        auto r_comma = s.find(",", r_dots);
        std::string_view range_end = s.substr(r_dots+2, r_comma-r_dots-2);
        std::cout << "range_end= " << range_end << std::endl;
        auto brk_end = s.find("]", r_comma);
        if(brk_end == std::string::npos)return {};
        std::string_view range_step = s.substr(r_comma+1, brk_end-r_comma-1);
        std::cout << "range_step= " << range_step << std::endl;

        g.m_range_begin = dev::stoui(range_begin);
        g.m_range_end = dev::stoui(range_end);
        g.m_range_step = dev::stoui(range_step);
        g.m_value = g.m_range_begin;
        return dev::var_generator{g};
    }
    
    return {};
};
