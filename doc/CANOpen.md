# CAN and CANOpen Documentation
# FDCAN Pins
PA11 FDCAN1_RX
PA12 FDCAN2_TX

Bitrate is controlled by setting time quota and prescaler accordingly:
Bitrate = CAN_CLK / (PSC * ( Seg1 + Seg2 +1) 

e.g for 125 kbps:
can clk = 16 MHz
prescaler =  16
nominal time seg 1 = 4
nominal time seg 1 = 2


## CANOpen
NMT Heartbeat Message:
can1 702 [1] 00 // 0x700 + $nodeid

Node-ID is set in sdev.c :
``` 
const struct co_sdev lpc17xx_sdev = {
	.id = 0x02,
```
Each tx-interval is in 0x1017 entries value:
```
 {
		.name = CO_SDEV_STRING("Producer heartbeat time"),
		.idx = 0x1017,
		.code = CO_OBJECT_VAR,
		.nsub = 1,
		.subs = (const struct co_ssub[]){{
			.name = CO_SDEV_STRING("Producer heartbeat time"),
			.subidx = 0x00,
			.type = CO_DEFTYPE_UNSIGNED16,
			.min = { .u16 = CO_UNSIGNED16_MIN },
			.max = { .u16 = CO_UNSIGNED16_MAX },
			.def = { .u16 = 0x00C8u },
			.val = { .u16 = 0x00C8u },
			.access = CO_ACCESS_RW,
			.pdo_mapping = 0,
			.flags = 0
		}}
	}	
```
