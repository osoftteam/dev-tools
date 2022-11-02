#include <gtest/gtest.h>
#include "dev-utils.h"
#include "tag_generator.h"

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
            dev::tag_generator_stringer sgf;
            std::visit(sgf, v);
//            std::cout << "   variant index= [" << v.index() << "] rule=" << sgf.str() << std::endl;
            ASSERT_EQ(sgf.str(), s);
        }
    }
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}