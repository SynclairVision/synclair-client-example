#include <gtest/gtest.h>
#include "../message-definitions/msg_defs.hpp"
#include "../message-definitions/message.hpp"

// Enhetstest för pack_get_parameters
TEST(MessageTest, PackGetParameters) {
    message msg;
    pack_get_parameters(msg, SYSTEM_STATUS);
    EXPECT_EQ(msg.message_type, SYSTEM_STATUS);
}

// Lägg till fler tester här...

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}




