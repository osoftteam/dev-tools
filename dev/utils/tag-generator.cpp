#include "tag-generator.h"

////////// set_tag_stat ////////////

void dev::set_tag_stat::update_stat(const std::string_view& s)
{
    ///@todo: don't want to do it...
    std::string str{s};
    auto i = m_stat.find(str);
    if(i == m_stat.end())
    {
        m_stat.emplace(s, 1);
    }
    else{
        ++(i->second);
    }
};

std::string dev::set_tag_stat::to_string()const
{
    std::string rv = "(";
    auto it = m_stat.cbegin();
    while(it != m_stat.cend())
    {
        rv += it->first;
        rv += ":";
        rv += dev::size_human(it->second, false);
        ++it;
        if(it != m_stat.cend())rv += ",";
    }
    
    rv += ")";
    return rv;
};

std::ostream& dev::operator<<(std::ostream& os, const dev::set_tag_stat& t)
{
    os << t.to_string();
    return os;
};

////////// range_tag_stat ////////////
void dev::range_tag_stat::update_stat(const std::string_view& s)
{
    auto val = dev::stoui(s);
    if(val > m_range_end)
    {
        m_range_end = val;
    }
    else
    {
        if(val < m_range_begin)
        {
            m_range_begin = val;
        }
        else if(m_range_begin == 0)
        {
            m_range_begin = val;
        }
    }
};

std::string dev::range_tag_stat::to_string()const
{
    std::string rv = "[";
    rv += std::to_string(m_range_begin);
    rv += "..";
    rv += std::to_string(m_range_end);
    rv += "]";
    return rv;
};

std::ostream& dev::operator<<(std::ostream& os, const dev::range_tag_stat& t)
{
    os << t.to_string();
    return os;
};



void dev::set_tag_generator::init()const
{
    m_it = m_data.cbegin();
};

std::string dev::set_tag_generator::next()const
{
    if(m_data.empty())return "";
    if(m_it == m_data.end())m_it = m_data.cbegin();
    auto rv = *m_it;
    ++m_it;
    return rv;
};

std::ostream& dev::operator<<(std::ostream& os, const dev::set_tag_generator& t)
{
    os << t.to_string();
    return os;
};

std::string dev::set_tag_generator::to_string()const
{
    std::string rv = "(";
    auto it = m_data.cbegin();
    while(it != m_data.cend())
    {
        rv += *it;
        ++it;
        if(it != m_data.cend())rv += ",";
    }
    rv += ")";
    return rv;
};

void dev::range_tag_generator::init()const
{
    m_value = m_range_begin;
};

std::string dev::range_tag_generator::next()const
{
    if(m_value >= m_range_end)m_value = m_range_begin;
    auto rv = std::to_string(m_value);
    m_value += m_range_step;
    return rv;
};

std::ostream& dev::operator<<(std::ostream& os, const dev::range_tag_generator& t)
{
    os << "[";
    os << std::to_string(t.m_range_begin);
    os << "..";
    os << std::to_string(t.m_range_end);
    os << ",";
    os << std::to_string(t.m_range_step);
    os << "]";
    return os;
};

std::string dev::range_tag_generator::to_string()const
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
        set_tag_generator g;
        
        size_t idx_b = 1;
        auto p = s.find_first_of(",)");
        while(p != std::string::npos){
            std::string s1 = std::string(s.substr(idx_b, p-idx_b));
            s1 = dev::trim(s1);
            if(!s1.empty())
            {
                g.m_data.push_back(s1);
            }
            idx_b = p + 1;
            p = s.find_first_of(",)", idx_b);
        }
        g.m_it = g.m_data.begin();
        return dev::var_generator{g};
    }
    else if(s[0] == '[')
    {
        range_tag_generator g;
        
        size_t idx_b = 1;
        auto r_dots = s.find("..", 1);
        if(r_dots == std::string::npos)return {};
        std::string_view range_begin = s.substr(idx_b, r_dots-idx_b);
        auto r_comma = s.find(",", r_dots);
        std::string_view range_end = s.substr(r_dots+2, r_comma-r_dots-2);
        auto brk_end = s.find("]", r_comma);
        if(brk_end == std::string::npos)return {};
        std::string_view range_step = s.substr(r_comma+1, brk_end-r_comma-1);

        g.m_range_begin = dev::stoui(range_begin);
        g.m_range_end = dev::stoui(range_end);
        g.m_range_step = dev::stoui(range_step);
        g.m_value = g.m_range_begin;
        return dev::var_generator{g};
    }
    
    return {};
};



bool dev::stat_tag_mapper::map_tag(size_t tag, const std::string_view& sv)
{
    auto i = m_stat.find(tag);
    if(i != m_stat.end())
    {
        std::visit([&sv](auto&& st){st.update_stat(sv);}, i->second);
    }
    return true;
};
