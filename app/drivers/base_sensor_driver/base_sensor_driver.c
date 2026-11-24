/* base_sensor_driver.c */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT base_sensor_driver

LOG_MODULE_REGISTER(base_sensor_driver, LOG_LEVEL_INF);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Dynamic data struct: add a custom parameter */
struct base_sensor_data {
	bool led_on;
	uint32_t blink_period_ms;   /* custom parameter */
};

/* ---- Standard sensor API ---- */
static int base_sensor_sample_fetch(const struct device *dev,
				    enum sensor_channel chan)
{
	struct base_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_VOLTAGE) {
		return -ENOTSUP;
	}

	data->led_on = !data->led_on;
	gpio_pin_set_dt(&led, data->led_on);
	return 0;
}

static int base_sensor_channel_get(const struct device *dev,
				   enum sensor_channel chan,
				   struct sensor_value *val)
{
	const struct base_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_VOLTAGE) {
		return -ENOTSUP;
	}
	val->val1 = data->led_on ? 1 : 0;
	val->val2 = 0;
	return 0;
}

/* ---- Custom extension API ---- */
/* Custom function: set blink period */
static int base_sensor_set_blink_period(const struct device *dev,
					uint32_t period_ms)
{
	struct base_sensor_data *data = dev->data;

	data->blink_period_ms = period_ms;
	LOG_INF("Blink period set to %u ms", period_ms);
	return 0;
}

/* Custom API structure */
struct base_sensor_driver_api {
	/* Inherit standard sensor API as the first member */
	struct sensor_driver_api sensor_api;
	/* Custom function pointer */
	int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};

/* Helper macro: cast device API to custom API and call */
static inline const struct base_sensor_driver_api *
base_sensor_get_api(const struct device *dev)
{
	return (const struct base_sensor_driver_api *)dev->api;
}

static inline int base_sensor_set_period(const struct device *dev, uint32_t ms)
{
	return base_sensor_get_api(dev)->set_blink_period(dev, ms);
}

/* Fill in custom API structure */
static const struct base_sensor_driver_api base_sensor_api = {
	.sensor_api = {
		.sample_fetch = base_sensor_sample_fetch,
		.channel_get = base_sensor_channel_get,
	},
	.set_blink_period = base_sensor_set_blink_period,
};

static int base_sensor_init(const struct device *dev)
{
	struct base_sensor_data *data = dev->data;

	if (!gpio_is_ready_dt(&led)) {
		return -ENODEV;
	}
	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) < 0) {
		return -EIO;
	}
	data->led_on = false;
	data->blink_period_ms = 1000; /* default value */
	return 0;
}

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