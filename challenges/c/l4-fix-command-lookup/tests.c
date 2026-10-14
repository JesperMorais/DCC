#include <string.h>

TEST(finds_a_command_given_as_a_literal) {
    EXPECT_EQ(command_opcode("status"), 3);
}

TEST(finds_a_command_typed_into_a_buffer) {
    char typed[16];
    strcpy(typed, "stop");
    EXPECT_EQ(command_opcode(typed), 2);
}

TEST(finds_every_command_in_the_table) {
    const char *names[] = {"start", "stop", "status", "reset"};
    for (int i = 0; i < 4; i++) {
        char typed[16];
        strcpy(typed, names[i]);
        EXPECT_EQ(command_opcode(typed), i + 1);
    }
}

TEST(finds_a_word_cut_out_of_a_longer_line) {
    char line[] = "reset now";
    line[5] = '\0';
    EXPECT_EQ(command_opcode(line), 4);
}

TEST(unknown_command_is_minus_one) {
    char typed[16];
    strcpy(typed, "halt");
    EXPECT_EQ(command_opcode(typed), -1);
}

TEST(matching_is_case_sensitive) {
    EXPECT_EQ(command_opcode("STOP"), -1);
}

TEST(a_prefix_is_not_a_match) {
    EXPECT_EQ(command_opcode("sta"), -1);
    EXPECT_EQ(command_opcode(""), -1);
}
