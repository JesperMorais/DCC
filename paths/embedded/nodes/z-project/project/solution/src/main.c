/*
 * The entry point. Keep main.c to main() only: the tests build every other
 * file in src/ into their own image (which has its own main), so anything
 * the node does must start without main's help.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensor_node, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("sensor node starting on %s", CONFIG_BOARD);
	LOG_INF("sampling every %d ms, reporting every %d ms", CONFIG_SENSOR_NODE_SAMPLE_MS,
		CONFIG_SENSOR_NODE_REPORT_MS);
	return 0;
}
