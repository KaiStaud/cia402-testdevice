#include <stdlib.h>
#include <stdio.h>
#include <lely/co/nmt.h>
#include <lely/co/sdev.h>
#include <lely/co/sdo.h>
#include <lely/co/time.h>
#include <lely/co/co.h>
#include <lely/bsp/can.h>

struct co_dev_t* co_create_dev(const struct co_sdev lpc17xx_sdev);
struct co_nmt_t* co_create_nmt(co_dev_t* dev,can_net_t* net);
struct can_net_t* co_add_indication_cb(co_dev_t *dev,co_nmt_t *nmt);