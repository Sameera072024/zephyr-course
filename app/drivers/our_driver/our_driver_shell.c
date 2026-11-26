// #include "zephyr/toolchain.h"
// #include <zephyr/shell/shell.h>

// static int cmd_channel_get_handler(const struct shell* sh, int argc, int argv)
// {
//     ARG_UNUSED(sh);
//     ARG_UNUSED(argc);
//     ARG_UNUSED(argv);
//     return 0;
// }

// SHELL_STATIC_SUBCMD_SET_CREATE(our_driver_subcmd, 
//     SHELL_CMD_ARG(channel_get, NULL, "Get channel of our driver", cmd_channel_get_handler, 1, 0),
//     SHELL_SUBCMD_SET_END,
// );

// SHELL_CMD_REGISTER(our_driver, &our_driver_subcmd, "Our driver set of cmds", NULL);

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>

/* 
 * Replace "base_sensor" with the actual devicetree node label 
 * of your LED sensor driver from L06 Task 1.
 */
#define SENSOR_NODE DT_NODELABEL(base_sensor_driver)

static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    const struct device *dev = DEVICE_DT_GET(SENSOR_NODE);

    if (!device_is_ready(dev)) {
        shell_error(sh, "Sensor device is not ready");
        return -ENODEV;
    }

    int ret = sensor_sample_fetch(dev);
    if (ret < 0) {
        shell_error(sh, "Failed to fetch sample: %d", ret);
        return ret;
    }

    shell_print(sh, "Sample fetched successfully.");
    return 0;
}

static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    const struct device *dev = DEVICE_DT_GET(SENSOR_NODE);

    if (!device_is_ready(dev)) {
        shell_error(sh, "Sensor device is not ready");
        return -ENODEV;
    }

    struct sensor_value val;
    
    /* Using SENSOR_CHAN_VOLTAGE as it matches your L06 channel_get implementation */
    int ret = sensor_channel_get(dev, SENSOR_CHAN_VOLTAGE, &val);
    if (ret < 0) {
        shell_error(sh, "Failed to read channel: %d", ret);
        return ret;
    }

    /* Prints the integer value (1 for LED ON, 0 for LED OFF) */
    shell_print(sh, "LED State (Channel Value): %d", val.val1);
    return 0;
}

static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    const struct device *dev = DEVICE_DT_GET(SENSOR_NODE);

    shell_print(sh, "Device Name: %s", dev->name);
    shell_print(sh, "Ready State: %s", device_is_ready(dev) ? "READY" : "NOT READY");

    return 0;
}

/* 
 * New handler: Set custom parameter 
 * e.g., sensorroot set 500
 */
 /* Custom API structure */
struct base_sensor_driver_api {
	/* Inherit standard sensor API as the first member */
	struct sensor_driver_api sensor_api;
	/* Custom function pointer */
	int (*set_blink_period)(const struct device *dev, uint32_t period_ms);
};

static inline int base_sensor_set_period(const struct device *dev, uint32_t ms)
{
	const struct base_sensor_driver_api *api =
		(const struct base_sensor_driver_api *)dev->api;
	return api->set_blink_period(dev, ms);
}

static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
    /* argc is 2 because: argv[0]="set", argv[1]="<value>" */
    const struct device *dev = DEVICE_DT_GET(SENSOR_NODE);

    if (!device_is_ready(dev)) {
        shell_error(sh, "Sensor device is not ready");
        return -ENODEV;
    }

    /* 1. Convert the string argument to an integer */
    char *endptr;
    long value = strtol(argv[1], &endptr, 10);

    /* 2. Validate the argument: check if it's a valid number */
    if (*endptr != '\0' || endptr == argv[1]) {
        shell_error(sh, "Error: '%s' is not a valid number.", argv[1]);
        return -EINVAL;
    }

    /* 3. Validate the argument: check if it's within range (e.g., 100ms to 5000ms) */
    if (value < 100 || value > 5000) {
        shell_error(sh, "Error: Value %ld out of range. Must be between 100 and 5000.", value);
        return -EINVAL;
    }

    /* 4. Call the custom extension API from L06 Task 2 */
    /* (Assuming you named it base_sensor_set_period) */
    int ret = base_sensor_set_period(dev, (uint32_t)value);
    
    if (ret < 0) {
        shell_error(sh, "Failed to set parameter: %d", ret);
        return ret;
    }

    shell_print(sh, "Success: Custom parameter set to %ld", value);
    return 0;
}

/* Create the sensorroot subcommands */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensorroot,
    SHELL_CMD(fetch, NULL, "Fetch a sample from the sensor", cmd_sensor_fetch),
    SHELL_CMD(read, NULL, "Read the channel value from the sensor", cmd_sensor_read),
    SHELL_CMD(info, NULL, "Print device name and ready state", cmd_sensor_info),
    /* Use SHELL_CMD_ARG to enforce argument count:
     * 1 mandatory arg (the value), 0 optional args */
    SHELL_CMD_ARG(set, NULL, "Set custom parameter (e.g., set 500)", cmd_sensor_set, 2, 0),
    
    SHELL_SUBCMD_SET_END
);

/* Register the root command */
SHELL_CMD_REGISTER(sensorroot, &sub_sensorroot, "Sensor root commands", NULL);