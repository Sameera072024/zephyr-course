#include "zephyr/drivers/sensor.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* The devicetree node identifier for the "led0" alias. */
#define LED_NODE DT_ALIAS(led2)

/* devicetree reference by node label identifier */
//#define LED_NODE DT_NODELABEL(green_led)

/* devicetree reference by node path identifier */
// Blue led: /leds/led_2
//#define LED_NODE DT_PATH(leds, led_2)

/* devicetree reference by alias */
//#define LED_NODE DT_ALIAS(warning_led)

/* Assignment l4-task1 - to use the app.overlay alias */
//#define LED_NODE DT_ALIAS(app_led)


void hello_world(void);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void hello_world(void)
{
	printk("hello world\n");
}

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

namespace  {
    void test() {
        const struct device* driver = DEVICE_DT_GET(DT_NODELABEL(our_driver0));
        struct sensor_value val;
        auto ret = sensor_channel_get(driver, SENSOR_CHAN_AMBIENT_TEMP, &val);
        LOG_INF("Channel ret %d", ret);
    }

}

/* Custom API declaration */
struct base_sensor_driver_api {
	struct sensor_driver_api sensor_api;
	int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};

static inline int base_sensor_set_period(const struct device *dev, uint32_t ms)
{
	const struct base_sensor_driver_api *api =
		(const struct base_sensor_driver_api *)dev->api;
	return api->set_blink_period(dev, ms);
}

int main(void)
{
    // bool led_state = true;

    // hello_world();

    // if (!gpio_is_ready_dt(&led)) return 0;

    // if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) return 0;

    // while (1) {
    //     if (gpio_pin_toggle_dt(&led) < 0) return 0;

    //     led_state = !led_state;
    //     LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
    //     //k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);
    //     /* Assignment l4-task1 - use kconfig app heartbeat period */
    //     k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);

    // }

    const struct device *led_sensor = DEVICE_DT_GET(DT_NODELABEL(base_sensor_driver));

    if (!device_is_ready(led_sensor)) {
		printk("Sensor device not ready\n");
		return 0;
	}

    struct sensor_value val;

	/* Call custom API: change blink period to 500ms */
	base_sensor_set_period(led_sensor, 500);

    while(1){
	    sensor_sample_fetch(led_sensor);                          /* LED turns on */
	    printk("LED state: %d\n", val.val1);                           /* Should print 1 */
        k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
    }
    return 0;
}
