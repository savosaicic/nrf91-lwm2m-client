/*
 * LwM2M IPSO Time Object (ID: 3333)
 *
 * Resources:
 *   5506 - Current Time  (R/W, Mandatory, time_t)
 *   5507 - Fractional Time (R, Optional, double)
 *   5750 - Application Type (R/W, Optional, string)
 */

#define LOG_MODULE_NAME lwm2m_obj_time
#define LOG_LEVEL       CONFIG_LWM2M_LOG_LEVEL

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <time.h>

/* Internal Zephyr LwM2M headers (not part of public API) */
#include "lwm2m_object.h"
#include "lwm2m_engine.h"

/* Object / resource IDs */
#define IPSO_TIME_OBJ_ID 3333

#define CURRENT_TIME_RID     5506
#define FRACTIONAL_TIME_RID  5507
#define APPLICATION_TYPE_RID 5750

#define TIME_MAX_ID        3 /* number of resources */
#define MAX_INSTANCE_COUNT 1
#define APP_TYPE_LEN       32

/* Resource instance count: 1 per resource (none are multi-instance) */
#define RESOURCE_INSTANCE_COUNT TIME_MAX_ID

/* ---- Per-instance data storage ---- */
static time_t current_time[MAX_INSTANCE_COUNT];
static double frac_time[MAX_INSTANCE_COUNT];
static char   app_type[MAX_INSTANCE_COUNT][APP_TYPE_LEN];

/* ---- Engine structures ---- */
static struct lwm2m_engine_obj time_obj;

static struct lwm2m_engine_obj_field fields[] = {
  OBJ_FIELD_DATA(CURRENT_TIME_RID, RW, TIME),
  OBJ_FIELD_DATA(FRACTIONAL_TIME_RID, R_OPT, FLOAT),
  OBJ_FIELD_DATA(APPLICATION_TYPE_RID, RW_OPT, STRING),
};

static struct lwm2m_engine_obj_inst inst[MAX_INSTANCE_COUNT];
static struct lwm2m_engine_res      res[MAX_INSTANCE_COUNT][TIME_MAX_ID];
static struct lwm2m_engine_res_inst res_inst[MAX_INSTANCE_COUNT]
                                            [RESOURCE_INSTANCE_COUNT];

/* ---- create_cb: called by lwm2m_create_object_inst() ---- */
static struct lwm2m_engine_obj_inst *time_create(uint16_t obj_inst_id)
{
  int avail = -1;
  int i = 0, j = 0;

  /* Find a free slot */
  for (int k = 0; k < MAX_INSTANCE_COUNT; k++) {
    if (!inst[k].obj) {
      avail = k;
      break;
    }
  }

  if (avail < 0) {
    LOG_ERR("No free Time object instance");
    return NULL;
  }

  /* Zero out the resource arrays for this slot */
  (void)memset(res[avail], 0, sizeof(res[avail]));
  (void)memset(res_inst[avail], 0, sizeof(res_inst[avail]));

  /* Bind resource 5506: Current Time (mandatory) */
  INIT_OBJ_RES_DATA(CURRENT_TIME_RID, res[avail], i, res_inst[avail], j,
                    &current_time[avail], sizeof(current_time[avail]));

  /* Bind resource 5507: Fractional Time (optional) */
  INIT_OBJ_RES_DATA(FRACTIONAL_TIME_RID, res[avail], i, res_inst[avail], j,
                    &frac_time[avail], sizeof(frac_time[avail]));

  /* Bind resource 5750: Application Type (optional) */
  INIT_OBJ_RES_DATA_LEN(APPLICATION_TYPE_RID, res[avail], i, res_inst[avail], j,
                        app_type[avail], APP_TYPE_LEN, 0);

  inst[avail].resources      = res[avail];
  inst[avail].resource_count = i;

  LOG_DBG("Create Time object instance: %d", obj_inst_id);
  return &inst[avail];
}

/* ---- init function: registers the object type ---- */
static int lwm2m_time_obj_init(void)
{
  time_obj.obj_id             = IPSO_TIME_OBJ_ID;
  time_obj.version_major      = 1;
  time_obj.version_minor      = 0;
  time_obj.is_core            = false;
  time_obj.fields             = fields;
  time_obj.field_count        = ARRAY_SIZE(fields);
  time_obj.max_instance_count = MAX_INSTANCE_COUNT;
  time_obj.create_cb          = time_create;

  lwm2m_register_obj(&time_obj);
  return 0;
}

/* Auto-register at boot using Zephyr's LwM2M init mechanism */
LWM2M_OBJ_INIT(lwm2m_time_obj_init);
