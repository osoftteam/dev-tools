#include <gtest/gtest.h>
#include "dev-utils.h"
#include "tag-generator.h"

TEST(set_generator, parse)
{
    dev::tag_generator_factory fct;
    std::vector<std::string> cfg = {"(AAPL,MSFT,IBM)", "(DAL)", "(CCJ,GOOG,META)",
        "[100..1000,200]", "[10..100,22]", "[1..20,2]"};
    for(const auto& s : cfg)
    {
        auto r = fct.produce_generator(s);
        ASSERT_EQ(r.has_value(), true);
        if(r)
        {
            auto& v = r.value();
            std::string s2;
            std::visit([&s2](auto&& g){s2 = g.to_string();}, v);
            ASSERT_EQ(s2, s);
        }
    }
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
