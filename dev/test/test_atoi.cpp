#include <gtest/gtest.h>
#include "dev-utils.h"

TEST(stoui, numbers)
{
    int idx = 0;
    for(size_t i = 11; i < 117700;){
        i += 357;
        ++idx;
        ASSERT_EQ(i, dev::stoui(std::to_string(i)));
    }
}


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
