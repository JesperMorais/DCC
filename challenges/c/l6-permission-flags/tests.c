#include <stdbool.h>

TEST(grant_adds_flags) {
    EXPECT_UEQ(perm_grant(0, PERM_READ), 0x1u);
    EXPECT_UEQ(perm_grant(PERM_READ, PERM_WRITE | PERM_EXEC), 0x7u);
    EXPECT_UEQ(perm_grant(PERM_WRITE, PERM_WRITE), 0x2u);   // granting twice is harmless
}

TEST(grant_drops_unknown_bits) {
    EXPECT_UEQ(perm_grant(0, 0xFFu), PERM_ALL);
    EXPECT_UEQ(perm_grant(PERM_READ, 0x10u), PERM_READ);
}

TEST(revoke_removes_only_the_given_flags) {
    EXPECT_UEQ(perm_revoke(PERM_ALL, PERM_WRITE), 0x5u);
    EXPECT_UEQ(perm_revoke(PERM_READ | PERM_EXEC, PERM_READ | PERM_EXEC), 0x0u);
    EXPECT_UEQ(perm_revoke(PERM_READ, PERM_WRITE), 0x1u);   // wasn't there: no change
}

TEST(has_all_needs_every_required_flag) {
    unsigned p = PERM_READ | PERM_WRITE;
    EXPECT_TRUE(perm_has_all(p, PERM_READ));
    EXPECT_TRUE(perm_has_all(p, PERM_READ | PERM_WRITE));
    EXPECT_FALSE(perm_has_all(p, PERM_READ | PERM_EXEC));
    EXPECT_TRUE(perm_has_all(p, 0));
    EXPECT_TRUE(perm_has_all(0, 0));
}

TEST(has_any_needs_at_least_one_flag) {
    unsigned p = PERM_READ | PERM_WRITE;
    EXPECT_TRUE(perm_has_any(p, PERM_READ | PERM_EXEC));
    EXPECT_FALSE(perm_has_any(p, PERM_EXEC));
    EXPECT_FALSE(perm_has_any(p, 0));
    EXPECT_FALSE(perm_has_any(0, PERM_ALL));
}

TEST(to_string_shows_rwx_labels) {
    char label[4] = "???";
    perm_to_string(PERM_ALL, label);
    EXPECT_STR_EQ(label, "rwx");
    perm_to_string(PERM_READ | PERM_EXEC, label);
    EXPECT_STR_EQ(label, "r-x");
    perm_to_string(PERM_WRITE, label);
    EXPECT_STR_EQ(label, "-w-");
    perm_to_string(0, label);
    EXPECT_STR_EQ(label, "---");
}
