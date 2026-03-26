#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/lwm2m.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(ecompass);

static const struct device *accel =
  DEVICE_DT_GET(DT_NODELABEL(lsm303agr_accel));
static const struct device *mag =
  DEVICE_DT_GET(DT_NODELABEL(lsm303agr_mag));

static struct k_work_delayable ecompass_work;

#define ECOMPASS_UPDATE_INTERVAL_S 10

/* x is internally allocated in the lwm2m engine */
static double accel_y = 0.0;
static double accel_z = 0.0;
static double mag_y   = 0.0;
static double mag_z   = 0.0;

static void ecompass_work_handler(struct k_work *work)
{
  struct sensor_value accel_xyz[3];
  struct sensor_value mag_xyz[3];
  int                 ret;

  ret = sensor_sample_fetch(accel);
  if (ret < 0) {
    LOG_ERR("Failed to fetch accel sample: %d", ret);
    goto reschedule;
  }
  ret = sensor_sample_fetch(mag);
  if (ret < 0) {
    LOG_ERR("Failed to fetch mag sample: %d", ret);
    goto reschedule;
  }

  ret = sensor_channel_get(accel, SENSOR_CHAN_ACCEL_XYZ, accel_xyz);
  if (ret < 0) {
    LOG_ERR("Failed to get accel data: %d", ret);
    goto reschedule;
  }
  ret = sensor_channel_get(mag, SENSOR_CHAN_MAGN_XYZ, mag_xyz);
  if (ret < 0) {
    LOG_ERR("Failed to get mag data: %d", ret);
    goto reschedule;
  }

  ret = lwm2m_set_f64(&LWM2M_OBJ(3313, 0, 5702),
                      sensor_value_to_double(&accel_xyz[0]));
  if (ret < 0) {
    LOG_ERR("Failed to set accel X: %d", ret);
  }
  ret = lwm2m_set_f64(&LWM2M_OBJ(3313, 0, 5703),
                      sensor_value_to_double(&accel_xyz[1]));
  if (ret < 0) {
    LOG_ERR("Failed to set accel Y: %d", ret);
  }
  ret = lwm2m_set_f64(&LWM2M_OBJ(3313, 0, 5704),
                      sensor_value_to_double(&accel_xyz[2]));
  if (ret < 0) {
    LOG_ERR("Failed to set accel Z: %d", ret);
  }

  ret = lwm2m_set_f64(&LWM2M_OBJ(3314, 0, 5702),
                      sensor_value_to_double(&mag_xyz[0]));
  if (ret < 0) {
    LOG_ERR("Failed to set mag X: %d", ret);
  }
  ret = lwm2m_set_f64(&LWM2M_OBJ(3314, 0, 5703),
                      sensor_value_to_double(&mag_xyz[1]));
  if (ret < 0) {
    LOG_ERR("Failed to set mag Y: %d", ret);
  }
  ret = lwm2m_set_f64(&LWM2M_OBJ(3314, 0, 5704),
                      sensor_value_to_double(&mag_xyz[2]));
  if (ret < 0) {
    LOG_ERR("Failed to set mag Z: %d", ret);
  }

reschedule:
  k_work_schedule(&ecompass_work, K_SECONDS(ECOMPASS_UPDATE_INTERVAL_S));
}

static int setup_accelerometer(void)
{
  int ret;

  ret = lwm2m_create_object_inst(&LWM2M_OBJ(3313, 0));
  if (ret < 0) {
    LOG_ERR("Failed to create Accelerometer instance: %d", ret);
    return ret;
  }

  ret = lwm2m_set_res_buf(&LWM2M_OBJ(3313, 0, 5703), &accel_y, sizeof(accel_y),
                          sizeof(accel_y), 0);
  if (ret < 0) {
    LOG_ERR("accel Y buf: %d", ret);
    return ret;
  }

  ret = lwm2m_set_res_buf(&LWM2M_OBJ(3313, 0, 5704), &accel_z, sizeof(accel_z),
                          sizeof(accel_z), 0);
  if (ret < 0) {
    LOG_ERR("accel Z buf: %d", ret);
    return ret;
  }

  return 0;
}

static int setup_magnetometer(void)
{
  int ret;

  ret = lwm2m_create_object_inst(&LWM2M_OBJ(3314, 0));
  if (ret < 0) {
    LOG_ERR("Failed to create Magnetometer instance: %d", ret);
    return ret;
  }

  ret = lwm2m_set_res_buf(&LWM2M_OBJ(3314, 0, 5703), &mag_y, sizeof(mag_y),
                          sizeof(accel_y), 0);
  if (ret < 0) {
    LOG_ERR("mag Y buf: %d", ret);
    return ret;
  }

  ret = lwm2m_set_res_buf(&LWM2M_OBJ(3313, 0, 5704), &mag_z, sizeof(mag_z),
                          sizeof(mag_z), 0);
  if (ret < 0) {
    LOG_ERR("mag Z buf: %d", ret);
    return ret;
  }

  return 0;
}

int setup_ecompass(void)
{
  int ret;

  if (!device_is_ready(accel)) {
    LOG_ERR("Accelerometer not ready");
    return -ENODEV;
  }
  if (!device_is_ready(mag)) {
    LOG_ERR("Magnetometer not ready");
    return -ENODEV;
  }

  ret = setup_accelerometer();
  if (ret < 0) {
    LOG_ERR("Failed to setup accelerometer: %d", ret);
    return ret;
  }
  ret = setup_magnetometer();
  if (ret < 0) {
    LOG_ERR("Failed to setup magnetometer: %d", ret);
    return ret;
  }

  k_work_init_delayable(&ecompass_work, ecompass_work_handler);
  k_work_schedule(&ecompass_work, K_NO_WAIT);

  return 0;
}
