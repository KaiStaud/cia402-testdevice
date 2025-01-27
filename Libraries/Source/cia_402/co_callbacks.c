#include <cia_402/co_callbacks.h>
#include <stdlib.h>

 int on_can_send(const struct can_msg *msg, void *data)
{
	(void)data;
	return can_send(msg, 1) == 1 ? 0 : -1;
}

 void on_nmt_cs(co_nmt_t *nmt, co_unsigned8_t cs, void *data)
{
	(void)data;

	switch (cs) {
	case CO_NMT_CS_START:
		// Reset the TIME indication function, since the service may
		// have been restarted.
		co_time_set_ind(co_nmt_get_time(nmt), &on_time, NULL);
		break;
	case CO_NMT_CS_STOP:
		break;
	case CO_NMT_CS_ENTER_PREOP:
		co_time_set_ind(co_nmt_get_time(nmt), &on_time, NULL);
		break;
	case CO_NMT_CS_RESET_NODE:
		// Initiate a system reset.
		exit(0);
		break;
	case CO_NMT_CS_RESET_COMM:
		break;
	}
}

 void on_time(co_time_t *time, const struct timespec *tp, void *data)
{
	(void)time;
	(void)data;

	// Update the wall clock, _not_ the monotonic clock used by the CAN
	// network.
	clock_settime(CLOCK_REALTIME, tp);
}

 co_unsigned32_t on_dn_2000_00(co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	assert(sub);
	assert(co_obj_get_idx(co_sub_get_obj(sub)) == 0x2000);
	assert(co_sub_get_subidx(sub) == 0x00);
	assert(req);
	// The data pointer can be used to pass user-specified data to the
	// callback function. It is the last argument passed to
	// co_sub_set_dn_ind().
	(void)data;

	co_unsigned32_t ac = 0;

	co_unsigned16_t type = co_sub_get_type(sub);//dd
	assert(type == CO_DEFTYPE_UNSIGNED32);

	// This callback is invoked for every SDO CAN frame. Unless the value is
	// too large to be held in memory (for example, during a firmware
	// update), it is more convenient to wait until the entire value is
	// received. This is what the following call is designed to do. It
	// returns -1 with ac == 0 if more CAN frames are pending.
	union co_val val;
	if (co_sdo_req_dn_val(req, type, &val, &ac) == -1)
		return ac;

	// Check if the value is valid.
/*
	if (val.u32 != 42) {
		ac = CO_SDO_AC_PARAM;
		goto error;
	}
*/
	// TODO: Do something with val.u32.
//	trace("Received SDO 0x2000:0 : val.u32=[%d]",val.u32);
	// Write the temporary value to the local object dictionary.
	co_sub_dn(sub, &val);
/*
error:
	co_val_fini(type, &val);
	return ac;
*/
return ac;
}

 co_unsigned32_t on_up_2001_00(const co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	assert(co_obj_get_idx(co_sub_get_obj(sub)) == 0x2001);
	assert(co_sub_get_subidx(sub) == 0x00);
	assert(req);
	(void)data;

	co_unsigned16_t type = co_sub_get_type(sub);
	assert(type == CO_DEFTYPE_UNSIGNED32);

	// TODO: Obtain value from somewhere.
	co_unsigned32_t val = 42;

	// Store the value in the send buffer.
	co_unsigned32_t ac = 0;
	co_sdo_req_up_val(req, type, &val, &ac);
	return ac;
}



 co_unsigned32_t on_dn_6040_00(co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	assert(sub);
	assert(co_obj_get_idx(co_sub_get_obj(sub)) == 0x6040);
	assert(co_sub_get_subidx(sub) == 0x00);
	assert(req);
	(void)data;

	co_unsigned32_t ac = 0;

	co_unsigned16_t type = co_sub_get_type(sub);
	assert(type == CO_DEFTYPE_UNSIGNED32);

	union co_val val;
	if (co_sdo_req_dn_val(req, type, &val, &ac) == -1)
	{
		return ac;
	}

	// TODO: Do something with val.u32.
//	trace("Received SDO 0x6040:0 : val.u32=[%d]",val.u32);
	// Write the temporary value to the local object dictionary.
	co_sub_dn(sub, &val);
	return ac;
}

 co_unsigned32_t on_up_6041_00(const co_sub_t *sub, struct co_sdo_req *req, void *data)
{
	assert(co_obj_get_idx(co_sub_get_obj(sub)) == 0x6041);
	assert(co_sub_get_subidx(sub) == 0x00);
	assert(req);
	(void)data;

	co_unsigned16_t type = co_sub_get_type(sub);
	assert(type == CO_DEFTYPE_UNSIGNED32);

	// TODO: Obtain value from somewhere.
	co_unsigned32_t val = 42;

	// Store the value in the send buffer.
	co_unsigned32_t ac = 0;
	co_sdo_req_up_val(req, type, &val, &ac);
	return ac;
}