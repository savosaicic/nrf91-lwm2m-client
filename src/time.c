#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/lwm2m.h>
#include <date_time.h>

LOG_MODULE_REGISTER(time_object);

static int64_t current_time = 0;

static void *time_read_cb(uint16_t obj_inst_id, uint16_t res_id,
                          uint16_t res_inst_id, size_t *data_len)
{
  int64_t ms;
  int     ret;

  ret = date_time_now(&ms);
  if (ret != 0) {
    LOG_ERR("Failed to get current date time: %d", ret);
    return NULL;
  }

  current_time = ms / 1000;

  *data_len = sizeof(current_time);

  return &current_time;
}

int setup_time_object(void)
{
  int ret;

  ret = lwm2m_create_object_inst(&LWM2M_OBJ(3333, 0));
  if (ret < 0 && ret != -EEXIST) {
    LOG_ERR("Failed to create Time object instance: %d", ret);
    return ret;
  }

  current_time = 0;
  ret          = lwm2m_set_res_buf(&LWM2M_OBJ(3333, 0, 5506), &current_time,
                                   sizeof(current_time), sizeof(current_time), 0);
  if (ret < 0) {
    LOG_ERR("Failed to set time buffer: %d", ret);
    return ret;
  }

  ret = lwm2m_register_read_callback(&LWM2M_OBJ(3333, 0, 5506), time_read_cb);
  if (ret < 0) {
    LOG_ERR("Failed to register time read callback: %d", ret);
    return ret;
  }

  return 0;
}
