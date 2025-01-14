#include <string.h>
#include "ring_buffer.h"
#include "can.h"
#include <stm32g4xx.h>
#include "main.h"
#include <lely/util/util.h>
#include "../co/co.h"
#define CAN_RX_SIZE	16
#define CAN_TX_SIZE	16
#define CAN_MSG_MAX_DATA_LEN       (8)

/* CAN masks for identifiers */
//#define CANID_MASK 0x07FF /*!< CAN standard ID mask */
#define FLAG_RTR   0x8000 /*!< RTR flag, part of identifier */


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
	Msg.ID = RxHeader.Identifier| (RxHeader.RxFrameType == FDCAN_REMOTE_FRAME ? CAN_FLAG_RTR : 0x00);

	if(RxHeader.RxFrameType == FDCAN_REMOTE_FRAME)
	{
		Msg.Type = CAN_REMOTE_MSG;
	}
	else
	{
	}
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

size_t can_send(const struct can_msg *ptr, size_t n)
{
	size_t i = 0;
	// Disable transmit interrupts to prevent a race condition with
	// CAN_IRQHandler().
	//Chip_CAN_DisableInt(LPC_CAN1, CAN_IER_TIE);
	for (; i < n; i++) {
		if (ptr[i].len > CAN_MAX_LEN)
			continue;

		// Convert the CAN frame to the LPC message format.
		CAN_MSG_T Msg;
		if (ptr[i].flags & CAN_FLAG_IDE) {
			Msg.ID = ptr[i].id & CAN_MASK_EID;
			Msg.ID |= CAN_EXTEND_ID_USAGE;
		} else {
			Msg.ID = ptr[i].id & CAN_MASK_BID;
		}
		Msg.Type = (ptr[i].flags & CAN_FLAG_RTR) ? CAN_REMOTE_MSG : 0;
		Msg.DLC = ptr[i].len;
		memcpy(Msg.Data, ptr[i].data, ptr[i].len);

		// Try to flush the buffer in a non-blocking way if it is full.
		if (RingBuffer_IsFull(&txring))
			can_flush();
		// Drop remaining messages if the buffer is full.
		if (!RingBuffer_Insert(&txring, &Msg))
			break;
	}
	// Try to send the messages we just added to the buffer.
	can_flush();
	// If any messages remain in the buffer, re-enable transmit interrupts
	// to let CAN_IRQHandler() handle them.
	if (!RingBuffer_IsEmpty(&txring))
	{
//		Chip_CAN_EnableInt(LPC_CAN1, CAN_IER_TIE);
	}
	return i;
}


//int can_send(const struct can_msg *msg, size_t n)
static void can_flush(void)
{
	CAN_MSG_T msg;

	while (RingBuffer_Pop(&txring, &msg))
	{
	const uint16_t CANID_MASK = 0x07FF;
	FDCAN_TxHeaderTypeDef pTxHeader;
	pTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	pTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	pTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	pTxHeader.IdType = FDCAN_STANDARD_ID;
	pTxHeader.MessageMarker = 0;
	pTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	if(msg.Type == CAN_REMOTE_MSG)
	{
		pTxHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	}
	else
	{
		pTxHeader.TxFrameType = FDCAN_DATA_FRAME;
	}
//    tx_hdr.Identifier = buffer->ident & CANID_MASK;
//    tx_hdr.TxFrameType = (buffer->ident & FLAG_RTR) ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME
	pTxHeader.TxFrameType = //msg->flags;

	pTxHeader.Identifier = msg.ID & CANID_MASK;

    switch (msg.DLC) {
        case 0:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_0;
            break;
        case 1:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_1;
            break;
        case 2:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_2;
            break;
        case 3:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_3;
            break;
        case 4:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_4;
            break;
        case 5:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_5;
            break;
        case 6:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_6;
            break;
        case 7:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_7;
            break;
        case 8:
        	pTxHeader.DataLength = FDCAN_DLC_BYTES_8;
            break;
        default: /* Hard error... */
            break;
    }

    // Prüfen, ob die Daten nicht NULL sind
    if (msg.Data == NULL) {
        printf("Keine Daten zum Drucken.\n");
    }
;

    // Puffer für die formatierte Ausgabe
    char data_str[256]; // Angenommene Puffergröße, die groß genug ist
    int offset = 0;

    // Formatieren der Daten in den String
    offset += sprintf(data_str + offset, "  ID: 0x%X", msg.ID);
    offset += sprintf(data_str + offset, "  Length: %d", msg.DLC);
    //offset += sprintf(data_str + offset, "  Flags: 0x%X\r\n", msg.);

    offset += sprintf(data_str + offset, "  Data: [ ");
    for (unsigned int i = 0; i < msg.DLC; ++i) {
        offset += sprintf(data_str + offset, "%02X ", msg.Data[i]); // Ausgabe der Daten im Hex-Format
    }
    sprintf(data_str + offset, "]"); // Füge das abschließende Newline hinzu


//	trace("sending CAN-Frame: %s", data_str);

	int e =  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &pTxHeader, msg.Data);
	if(e != HAL_OK){
	    RTC_DateTypeDef gDate;
	    RTC_TimeTypeDef gTime;
	    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
	    //Display time Format: hh:mm:ss
	    trace("[ %02d:%02d:%02d ] failed sending CAN-Frame",gTime.Hours, gTime.Minutes, gTime.Seconds);
	}
	}
}
