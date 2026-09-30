#include <stdbool.h>

TEST(ignores_case_spaces_and_punctuation) {
    EXPECT_TRUE(is_loose_palindrome("A man, a plan, a canal: Panama!"));
}

TEST(mixed_case_word) {
    EXPECT_TRUE(is_loose_palindrome("Racecar"));
}

TEST(rejects_a_non_palindrome) {
    EXPECT_FALSE(is_loose_palindrome("palindrome"));
    EXPECT_FALSE(is_loose_palindrome("ab"));
}

TEST(rejects_when_only_the_middle_differs) {
    EXPECT_FALSE(is_loose_palindrome("Was it a car or a dog I saw?"));
}

TEST(empty_string_is_a_palindrome) {
    EXPECT_TRUE(is_loose_palindrome(""));
}

TEST(no_letters_at_all_is_a_palindrome) {
    char buf[] = "?! 42 ...";
    EXPECT_TRUE(is_loose_palindrome(buf));
}

TEST(single_letter_surrounded_by_junk) {
    char buf[] = "--x!";
    EXPECT_TRUE(is_loose_palindrome(buf));
}
