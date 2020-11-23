/*
Copyright 2020 <wanzp@thundersoft.com>
*/

#include "gtest/gtest/gtest.h"

#include "common.h"

TEST(client, client_start)
{
    const char *argv[]= { "--ip", "10.0.28.187", "--port", "9527" };
    /* const char *argv[]= { "--ip", "172.17.0.2", "--port", "9527" }; */
    int argc = sizeof(argv) / sizeof(char *);

    EXPECT_GE(client_start(argc, (char **)argv), 0);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
