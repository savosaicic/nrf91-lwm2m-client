#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/lwm2m.h>

LOG_MODULE_REGISTER(temp_sensor);

static double temperature = 20.0;
static double temp_min    = -21.0;
static double temp_max    = 42.0;

static struct k_work_delayable temp_work;

#define TEMP_UPDATE_INTERVAL_S 10

static void temp_work_handler(struct k_work *work)
{
  temperature += 0.5;
  if (temperature > 30.0) {
    temperature = 20.0;
  }

  int ret = lwm2m_set_f64(&LWM2M_OBJ(3303, 0, 5700), temperature);
  if (ret < 0) {
    LOG_ERR("Failed to update temperature: %d", ret);
  } else {
    LOG_DBG("Temperature updated: %.1f C", temperature);
  }

  k_work_schedule(&temp_work, K_SECONDS(TEMP_UPDATE_INTERVAL_S));
}

int setup_temperature_sensor(void)
{
  int ret;

  ret = lwm2m_create_object_inst(&LWM2M_OBJ(3303, 0));
  if (ret < 0 && ret != -EEXIST) {
    LOG_ERR("Failed to create Temperature object instance: %d", ret);
    return ret;
  }

  ret = lwm2m_set_f64(&LWM2M_OBJ(3303, 0, 5603), temp_min);
  if (ret < 0) {
    LOG_ERR("Failed to set temp min range: %d", ret);
    return ret;
  }

  ret = lwm2m_set_f64(&LWM2M_OBJ(3303, 0, 5604), temp_max);
  if (ret < 0) {
    LOG_ERR("Failed to set temp max range: %d", ret);
    return ret;
  }

  ret = lwm2m_set_string(&LWM2M_OBJ(3303, 0, 5701), "Celsius");
  if (ret < 0) {
    LOG_ERR("Failed to set sensor units: %d", ret);
    return ret;
  }

  k_work_init_delayable(&temp_work, temp_work_handler);
  k_work_schedule(&temp_work, K_NO_WAIT);

  return 0;
}
