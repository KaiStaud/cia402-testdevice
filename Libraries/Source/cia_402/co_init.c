#include <cia_402/co_init.h>
#include <cia_402/co_callbacks.h>


struct co_dev_t* co_create_dev(const struct co_sdev lpc17xx_sdev)
{
	// Create a dynamic object dictionary from the static object dictionary.
	struct co_dev_t* dev = co_dev_create_from_sdev(&lpc17xx_sdev);
	assert(dev);
    return dev;
}

struct co_nmt_t* co_create_nmt(co_dev_t* dev,can_net_t* net)
{
	// Create the CANopen NMT service.
	struct co_nmt_t* nmt = co_nmt_create(net, dev);
	assert(nmt);
    return nmt;
}
struct can_net_t* co_add_indication_cb(co_dev_t *dev,co_nmt_t *nmt)
{
	struct timespec now;
	// Initialize the CAN network interface.
	struct can_net_t* net = can_net_create();
	assert(net);
	can_net_set_send_func(net, &on_can_send, NULL);

	// Initialize the CAN network clock. 
//	struct timespec now = { 0, 0 };
	clock_gettime(1, &now);
	can_net_set_time(net, &now);

//... Todo 

	// Start the NMT service by resetting the node.
	co_nmt_cs_ind(nmt, CO_NMT_CS_RESET_NODE);

	// Set the NMT indication function _after_ the initial reset; otherwise
	// we create a reset loop.
	co_nmt_set_cs_ind(nmt, &on_nmt_cs, NULL);

	// Set the TIME indication function. This can only be done when the TIME
	// service is active.
	co_time_set_ind(co_nmt_get_time(nmt), &on_time, NULL);

	// Set the download (SDO write) indication function for sub-object
	// 2000:01.
	co_sub_set_dn_ind(co_dev_find_sub(dev, 0x2000, 0x00), &on_dn_2000_00,
			NULL);

	// Set the upload (SDO read) indication function for sub-object 2001:01.
	co_sub_set_up_ind(co_dev_find_sub(dev, 0x2001, 0x00), &on_up_2001_00,
			NULL);


	// Set the download (SDO write) indication function for sub-object
	// 2000:01.
	co_sub_set_dn_ind(co_dev_find_sub(dev, 0x6040, 0x00), &on_dn_6040_00,
			NULL);

	// Set the upload (SDO read) indication function for sub-object 2001:01.
	co_sub_set_up_ind(co_dev_find_sub(dev, 0x6041, 0x00), &on_up_6041_00,
			NULL);
}