#include <lely/co/nmt.h>
#include <lely/co/sdev.h>
#include <lely/co/sdo.h>
#include <lely/co/time.h>
#include <lely/co/co.h>
#include <lely/bsp/can.h>

 int on_can_send(const struct can_msg *msg, void *data);
 void on_nmt_cs(co_nmt_t *nmt, co_unsigned8_t cs, void *data);
 void on_time(co_time_t *time, const struct timespec *tp, void *data);
 co_unsigned32_t on_dn_2000_00(co_sub_t *sub, struct co_sdo_req *req,void *data);
 co_unsigned32_t on_up_2001_00(const co_sub_t *sub,struct co_sdo_req *req, void *data);
 co_unsigned32_t on_dn_6040_00(co_sub_t *sub, struct co_sdo_req *req,void *data);
 co_unsigned32_t on_up_6041_00(const co_sub_t *sub,struct co_sdo_req *req, void *data);