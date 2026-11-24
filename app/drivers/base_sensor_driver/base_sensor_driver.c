/* base_sensor_driver.c */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT base_sensor_driver

LOG_MODULE_REGISTER(base_sensor_driver, LOG_LEVEL_INF);

/* Get LED0 device from devicetree */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Driver private data: stores the "sample" state, i.e., LED state */
struct base_sensor_data {
	bool led_on;
};

/* sample_fetch callback: "Fetch" a sample, actually toggles LED */
static int base_sensor_sample_fetch(const struct device *dev,
				    enum sensor_channel chan)
{
	struct base_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_VOLTAGE) {
		return -ENOTSUP;
	}

	/* Toggle LED as the sample action */
	data->led_on = !data->led_on;
	gpio_pin_set_dt(&led, data->led_on);

	LOG_INF("Sample fetched, LED %s", data->led_on ? "ON" : "OFF");
	return 0;
}

/* channel_get callback: "Read" the channel, returns the current LED state */
static int base_sensor_channel_get(const struct device *dev,
				   enum sensor_channel chan,
				   struct sensor_value *val)
{
	const struct base_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_VOLTAGE) {
		return -ENOTSUP;
	}

	/* Encode LED state as sensor value: 1 for on, 0 for off */
	val->val1 = data->led_on ? 1 : 0;
	val->val2 = 0;
	return 0;
}

/* Fill in the sensor driver API structure */
static const struct sensor_driver_api base_sensor_api = {
	.sample_fetch = base_sensor_sample_fetch,
	.channel_get = base_sensor_channel_get,
};

/* Driver initialization function */
static int base_sensor_init(const struct device *dev)
{
	struct base_sensor_data *data = dev->data;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) {
		LOG_ERR("Failed to configure LED pin");
		return -EIO;
	}

	data->led_on = false;
	LOG_INF("Base sensor driver initialized");
	return 0;
}

/* Driver instance definition */
static struct base_sensor_data base_sensor_data_0;

#define BASE_SENSOR_DEFINE(inst)                                        \
    static struct base_sensor_data base_sensor_data_##inst;             \
    DEVICE_DT_INST_DEFINE(inst,                                         \
                          base_sensor_init,                             \
                          NULL,                                         \
                          &base_sensor_data_##inst,                     \
                          NULL,                                         \
                          POST_KERNEL,                                  \
                          90,                  \
                          &base_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(BASE_SENSOR_DEFINE)