#include "zephyr/device.h"
#include "zephyr/devicetree.h"
#include "zephyr/init.h"
#include "zephyr/logging/log.h"
#include "zephyr/logging/log_core.h"
#include <zephyr/drivers/sensor.h>

#define DT_DRV_COMPAT our_driver

LOG_MODULE_REGISTER(our_driver, LOG_LEVEL_INF);

static int channel_get_my_impl(const struct device *dev,
				    enum sensor_channel chan,
				    struct sensor_value *val)
{

    LOG_INF("Hello from channel Get. channel %d", chan);
    return 0;

}

static DEVICE_API(sensor, api_iomico_lecture) = {
    .channel_get = channel_get_my_impl,
    
};

//Init fn
static int init(const struct device* dev)
{
    LOG_INF("Device initialized");
    return 0;
}

#define DEV_INST(inst) DEVICE_DT_INST_DEFINE(inst, init, NULL, NULL, NULL, POST_KERNEL, 80, &api_iomico_lecture);

DT_INST_FOREACH_STATUS_OKAY(DEV_INST);