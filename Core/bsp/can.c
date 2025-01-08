#include <string.h>
#include "ring_buffer.h"
#include "can.h"
#include <stm32g4xx.h>
#include "main.h"
#include <lely/util/util.h>

#define CAN_RX_SIZE	16
#define CAN_TX_SIZE	16
#define CAN_MSG_MAX_DATA_LEN       (8)

/** Remote Message */
#define CAN_REMOTE_MSG         ((uint32_t) (1 << 0))

/** Message use Extend ID*/
#define CAN_EXTEND_ID_USAGE     ((uint32_t) (1 << 30))

typedef struct						/*!< Message structure */
{
	uint32_t ID;					/*!< Message Identifier. If 30th-bit is set, this is 29-bit ID, othewise 11-bit ID */
	uint32_t Type;					/*!< Message Type. which can include: - CAN_REMOTE_MSG type*/
	uint32_t DLC;					/*!< Message Data Length: 0~8 */
	uint8_t  Data[CAN_MSG_MAX_DATA_LEN];/*!< Message Data */
} CAN_MSG_T;


static CAN_MSG_T rxbuff[CAN_RX_SIZE];
static CAN_MSG_T txbuff[CAN_TX_SIZE];

static RINGBUFF_T rxring;
static RINGBUFF_T txring;

static void can_flush(void);

CAN_MSG_T prv_read_can_received_msg(FDCAN_HandleTypeDef* hfdcan, uint32_t fifo, uint32_t fifo_isrs)
{

	CAN_MSG_T rcvMsg;
//    CO_CANrx_t* buffer = NULL; /* receive message buffer from CO_CANmodule_t object. */
//    uint16_t index;            /* index of received message */
//    uint8_t messageFound = 0;

    static FDCAN_RxHeaderTypeDef rx_hdr;
    /* Read received message from FIFO */
    if (HAL_FDCAN_GetRxMessage(hfdcan, fifo, &rx_hdr, rcvMsg.Data) != HAL_OK) {
        return;
    }
    /* Setup identifier (with RTR) and length */
    rcvMsg.ID = rx_hdr.Identifier | (rx_hdr.RxFrameType == FDCAN_REMOTE_FRAME ? CAN_FLAG_RTR : 0x00);
    switch (rx_hdr.DataLength) {
        case FDCAN_DLC_BYTES_0:
            rcvMsg.DLC = 0;
            break;
        case FDCAN_DLC_BYTES_1:
            rcvMsg.DLC = 1;
            break;
        case FDCAN_DLC_BYTES_2:
            rcvMsg.DLC = 2;
            break;
        case FDCAN_DLC_BYTES_3:
            rcvMsg.DLC = 3;
            break;
        case FDCAN_DLC_BYTES_4:
            rcvMsg.DLC = 4;
            break;
        case FDCAN_DLC_BYTES_5:
            rcvMsg.DLC = 5;
            break;
        case FDCAN_DLC_BYTES_6:
            rcvMsg.DLC = 6;
            break;
        case FDCAN_DLC_BYTES_7:
            rcvMsg.DLC = 7;
            break;
        case FDCAN_DLC_BYTES_8:
            rcvMsg.DLC = 8;
            break;
        default:
            rcvMsg.DLC = 0;
            break; /* Invalid length when more than 8 */
    }
}

/**
 * \brief           Rx FIFO 0 callback.
 * \param[in]       hfdcan: pointer to an FDCAN_HandleTypeDef structure that contains
 *                      the configuration information for the specified FDCAN.
 * \param[in]       RxFifo0ITs: indicates which Rx FIFO 0 interrupts are signaled.
 */
/*
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan, uint32_t RxFifo0ITs) {
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
        prv_read_can_received_msg(hfdcan, FDCAN_RX_FIFO0, RxFifo0ITs);
		CAN_MSG_T Msg;
        RingBuffer_Insert(&rxring, &Msg);

    }
}
*/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	  FDCAN_RxHeaderTypeDef RxHeader;
	  uint8_t RxData[8];
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
      BSP_LED_Toggle(LED_GREEN);
	  /* Retrieve Rx messages from RX FIFO0 */
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
    {
    Error_Handler();
    }
	CAN_MSG_T Msg;
	Msg.DLC = RxHeader.DataLength;
	Msg.ID = RxHeader.Identifier;
	//Msg.Type =
	for(int i=0;i<8;i++)
	{
	    Msg.Data[i] = RxData[i];

	}
	RingBuffer_Insert(&rxring, &Msg);

  }
}
void can_init(int bitrate)
{
	RingBuffer_Init(&rxring, rxbuff, sizeof(CAN_MSG_T), CAN_RX_SIZE);
	RingBuffer_Init(&txring, txbuff, sizeof(CAN_MSG_T), CAN_TX_SIZE);
}

void can_fini(void)
{
}

size_t can_recv(struct can_msg *ptr, size_t n)
{
	size_t i = 0;
	for (; i < n; i++) {
		CAN_MSG_T Msg;
		if (!RingBuffer_Pop(&rxring, &Msg))
			break;

		// Convert the message to the LCI CAN frame format.
		ptr[i] = (struct can_msg)CAN_MSG_INIT;
		if (Msg.ID & CAN_EXTEND_ID_USAGE) {
			ptr[i].id = Msg.ID & CAN_MASK_EID;
			ptr[i].flags |= CAN_FLAG_IDE;
		} else {
			ptr[i].id = Msg.ID & CAN_MASK_BID;
		}
		if (Msg.Type == CAN_REMOTE_MSG)
			ptr[i].flags |= CAN_FLAG_RTR;
		ptr[i].len = MIN(Msg.DLC, CAN_MAX_LEN);
		memcpy(ptr[i].data, Msg.Data, ptr[i].len);
	}
	return i;
}

static void
can_flush(void)
{
	CAN_MSG_T Msg;

	while (RingBuffer_Pop(&txring, &Msg))
	{
		struct can_msg msg;
		msg.id = Msg.ID;
		msg.len = Msg.DLC;
		prv_can_send(&msg,&Msg.Data);

	}
}
