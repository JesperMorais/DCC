#include <zephyr/kernel.h>

TEST(a_card_wakes_the_reader_within_1ms) {
    sim_irq_at(23, "card", card_isr);
    sim_run(30);
    EXPECT_EQ(sim_first_run_at_or_after("reader", 23), 23);
    EXPECT_EQ(door_log.cards, 1);
}

TEST(back_to_back_cards_are_all_counted) {
    /* three cards 1 ms apart, but each takes 2 ms to process */
    sim_irq_at(10, "card", card_isr);
    sim_irq_at(11, "card", card_isr);
    sim_irq_at(12, "card", card_isr);
    sim_run(30);
    EXPECT_EQ(door_log.cards, 3);
}

TEST(silence_is_a_timeout_not_a_card) {
    sim_run(350);
    EXPECT_EQ(door_log.cards, 0);
    EXPECT_EQ(door_log.timeouts, 3); /* at 101, 202 and 303: a relative K_MSEC(100) waits 100 ms + 1 tick */
}

TEST(a_card_restarts_the_timeout_window) {
    sim_irq_at(60, "card", card_isr);
    sim_run(150);
    /* the wait that started at 0 ended with a card at 60, so no timeout at 100 */
    EXPECT_EQ(door_log.timeouts, 0);
    EXPECT_EQ(door_log.cards, 1);
}

TEST(nested_logging_does_not_deadlock) {
    sim_irq_at(20, "card", card_isr);
    sim_run(30);
    EXPECT_EQ(door_log.cards, 1);
    EXPECT_EQ(door_log.updates, 1); /* log_card() -> log_touch() ran under the same lock */
    EXPECT_FALSE(sim_deadlocked());
}

TEST(the_lock_holder_inherits_priority_during_an_upload) {
    /* cloud (prio 10) locks the log at 50 for 5 ms of work (it unlocks at 57, after being preempted). A card at 51 makes the reader (prio 2)
     * want the lock at 53, and ui (prio 6) wakes at 52 to burn 20 ms. */
    sim_irq_at(51, "card", card_isr);
    sim_run(60);
    EXPECT_EQ(door_log.cards, 1); /* without inheritance, ui would delay this to ~77 ms */
    EXPECT_EQ(sim_max_priority_seen("cloud"), sim_max_priority_seen("reader"));
}
