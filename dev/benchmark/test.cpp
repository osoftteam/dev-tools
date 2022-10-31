#include "dev-utils.h"


void generate_strings()
{
    std::string cfg_file = "sample.properties";
    auto r = dev::read_config(cfg_file);
    if(r)
    {
        dev::print_pairs(std::begin(r.value()), std::end(r.value()), "->");
    }
    else
    {
        std::cout << "failed tor read file [" << cfg_file << "]\n";
    }
}

int main(int, char**)
{
    generate_strings();
}
