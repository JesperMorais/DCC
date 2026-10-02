#include "FreeRTOS.h"
#include "sim.h"
#include "task.h"

/* The power manager (not your code) also pokes the IMU task directly. */
static void power_task(void *pvParameters) {
    (void)pvParameters;
    vTaskDelay(40);
    sim_mark("motion wake x3");
    for (int i = 0; i < 3; i++) xTaskNotifyGive(g_imu_task);
    vTaskDelete(NULL);
}

TEST(one_interrupt_gives_one_sample_in_the_same_tick) {
    app_main();
    sim_irq_at(10, "EXTI0", EXTI0_IRQHandler);
    sim_run(30);
    EXPECT_EQ(g_samples, 1);
    EXPECT_EQ(g_sample_tick[0], 10);
}

TEST(a_burst_of_five_interrupts_gives_five_samples) {
    app_main();
    for (int i = 0; i < 5; i++) sim_irq_at(20, "EXTI0", EXTI0_IRQHandler);
    sim_run(40);
    EXPECT_EQ(g_samples, 5); /* a binary semaphore, or pdTRUE that handles one, gives 1 */
    EXPECT_EQ(g_sample_tick[0], 20);
    EXPECT_EQ(g_sample_tick[4], 24);
}

TEST(direct_notifications_from_another_task_count_too) {
    app_main();
    xTaskCreate(power_task, "power", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    sim_run(60);
    EXPECT_EQ(g_samples, 3); /* only works if imu waits on its notification */
}

TEST(events_during_a_read_are_not_lost) {
    app_main();
    sim_irq_at(30, "EXTI0", EXTI0_IRQHandler);
    sim_irq_at(30, "EXTI0", EXTI0_IRQHandler);
    sim_irq_at(31, "EXTI0", EXTI0_IRQHandler); /* arrives while the first read is running */
    sim_irq_at(31, "EXTI0", EXTI0_IRQHandler);
    sim_run(50);
    EXPECT_EQ(g_samples, 4);
}

TEST(silence_is_detected_by_the_take_timeout) {
    app_main();
    sim_run(120);
    EXPECT_EQ(g_timeouts, 2); /* at 50 and 100 */
    EXPECT_EQ(g_samples, 0);
}

TEST(each_event_restarts_the_silence_timeout) {
    app_main();
    sim_irq_at(30, "EXTI0", EXTI0_IRQHandler);
    sim_run(110);
    EXPECT_EQ(g_samples, 1);
    EXPECT_EQ(g_timeouts, 1); /* only at 81: 50 ms after the read finished at 31 */
}
