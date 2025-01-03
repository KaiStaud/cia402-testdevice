/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// #define LELY_NO_THREADS 1
#include <stdlib.h>
#include <lely/co/nmt.h>
#include <lely/co/sdev.h>
#include <lely/co/sdo.h>
#include <lely/co/time.h>
#include "co/co.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;
FDCAN_HandleTypeDef hfdcan1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
int can_recv(struct can_msg *ptr, size_t n);

static int on_can_send(const struct can_msg *msg, void *data);
static void on_nmt_cs(co_nmt_t *nmt, co_unsigned8_t cs, void *data);
static void on_time(co_time_t *time, const struct timespec *tp, void *data);
static co_unsigned32_t on_dn_2000_00(co_sub_t *sub, struct co_sdo_req *req,
		void *data);
static co_unsigned32_t on_up_2001_00(const co_sub_t *sub,
		struct co_sdo_req *req, void *data);

// Generated by `dcf2c --no-strings lpc17xx.dcf lpc17xx_sdev -o src/sdev.c`
extern const struct co_sdev lpc17xx_sdev;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int ticks = 0,ticks_ms=0,ticks_second=0,ticks_minute=0,ticks_hour=0;

void
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
/* Update System Time
 * A tick is generated every ns
 */
//	HAL_GPIO_TogglePin(DBG_GPIO_Port, DBG_Pin);
	const int ticks_per_ms = 1000*1000;
	const int ticks_per_second = 1000;
	const int ticks_per_minute = 60;
	const int ticks_per_hour= 60;

	ticks++;
	ticks_ms = ticks;
/*
	if(ticks >= ticks_per_ms){
		ticks_ms++;
		ticks = 0;
	}
*/
	if(ticks_ms >= ticks_per_second){
		ticks_second++;
		ticks_ms=0;
		ticks = 0;
	}
	if(ticks_second >= ticks_per_minute){
		ticks_minute++;
		ticks_second=0;
	}
	if(ticks_minute >= ticks_per_hour){
		ticks_hour++;
		ticks_second=0;
	}
}

struct hw_time clock_get_hw_time()
{
struct hw_time tm;
tm.ticks_hour = ticks_hour;
tm.ticks_minute =ticks_minute;
tm.ticks_second = ticks_second;
tm.ticks_ms = ticks_ms;
tm.ticks_ns = ticks_ms * 1000* 1000;
return tm;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_FDCAN1_Init();
  MX_TIM7_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim7);
  /* USER CODE END 2 */

  /* Initialize led */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* USER CODE BEGIN BSP */
  /* Set CAN Filter, otherwise all messages will be filtered out */

  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT,
                                   FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE)
      != HAL_OK) {
      return -1;
  }

  if (HAL_FDCAN_ActivateNotification(&hfdcan1,
                                     0 | FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE
                                         | FDCAN_IT_TX_COMPLETE | FDCAN_IT_TX_FIFO_EMPTY | FDCAN_IT_BUS_OFF
                                         | FDCAN_IT_ARB_PROTOCOL_ERROR | FDCAN_IT_DATA_PROTOCOL_ERROR
                                         | FDCAN_IT_ERROR_PASSIVE | FDCAN_IT_ERROR_WARNING,
                                     0xFFFFFFFF)
      != HAL_OK) {
      return -1;
  }

  /* Put CAN module in normal mode */
      if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
      {
    	  return -1;
      }

   /* Start RTC, otherwise CANOpen won't generate Data */
  /* -- Sample board code to send message over COM1 port ---- */
  //printf("Welcome to STM32 world !\n\r");
  const char* buf = "\033c Welcome to lely tester!\n\r";
  HAL_UART_Transmit(&huart2, buf, 28, 10);

  /* -- Sample board code to switch on led ---- */
  BSP_LED_On(LED_GREEN);

	// Initialize the CAN network interface.
	can_net_t *net = can_net_create();
	assert(net);
	can_net_set_send_func(net, &on_can_send, NULL);

	// Initialize the CAN network clock. We use the monotonic clock, since
	// its reference will not be changed by clock_settime().
	struct timespec now = { 0, 0 };
	clock_gettime(1, &now);
	can_net_set_time(net, &now);

	// Create a dynamic object dictionary from the static object dictionary.
	co_dev_t *dev = co_dev_create_from_sdev(&lpc17xx_sdev);
	assert(dev);

	// Create the CANopen NMT service.
	co_nmt_t *nmt = co_nmt_create(net, dev);
	assert(nmt);

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

//	co_nmt_hb_set_st(hb, st);
  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* -- Sample board code for User push-button in interrupt mode ---- */
    if (BspButtonState == BUTTON_PRESSED)
    {
      /* Update button state */
      BspButtonState = BUTTON_RELEASED;
      /* -- Sample board code to toggle led ---- */
      BSP_LED_Toggle(LED_GREEN);

      /* ..... Perform your action ..... */
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	// Update the CAN network clock.
	clock_gettime(1, &now);

	struct tm *tm_info;
    char buffer[20];

    // Aktuelle Zeit holen
    tm_info = localtime(&now.tv_sec);

    // Zeit formatieren: hh:mm:ss:ms
/*
    sprintf(buffer, "%02d:%02d:%02d:%03ld",
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec,
            now.tv_nsec / 1000000); // Nanosekunden in Millisekunden umwandeln
*/

    RTC_DateTypeDef gDate;
    RTC_TimeTypeDef gTime;
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
    //Display time Format: hh:mm:ss
    //trace("%02d:%02d:%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);

	can_net_set_time(net, &now);

	// Process any received CAN frames.
	struct can_msg msg;
	while (can_recv(&msg, 1))
		can_net_recv(net, &msg);

	// TODO: Update object dictionary.
/*
	struct can_msg heartbeat;
	heartbeat.id = 0x702;
	heartbeat.flags = 0;
	heartbeat.len = 1;
	can_send(&heartbeat, 0);
*/
  }
	co_nmt_destroy(nmt);
	co_dev_destroy(dev);
	can_net_destroy(net);

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_EXTERNAL_LOOPBACK;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 16;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 4;
  hfdcan1.Init.NominalTimeSeg2 = 3;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.SubSeconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 32;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DBG_GPIO_Port, DBG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : DBG_Pin */
  GPIO_InitStruct.Pin = DBG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(DBG_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
int can_recv(struct can_msg *ptr, size_t n)
{
	FDCAN_RxHeaderTypeDef pRxHeader ={};
	uint8_t* data;
	HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &pRxHeader, data);//ptr->data);
//	trace("received CAN-Frame: ID=%d, %d bytes", pRxHeader->Identifier,pRxHeader->DataLength);

/*
	ptr->flags = pRxHeader->RxFrameType;
	ptr->id = pRxHeader->Identifier;
	ptr->len = pRxHeader->DataLength;
*/
	return pRxHeader.DataLength;
}



int can_send(const struct can_msg *msg, void *data)
{
const uint16_t CANID_MASK = 0x07FF;
	FDCAN_TxHeaderTypeDef pTxHeader;
	pTxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	pTxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	pTxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	pTxHeader.IdType = FDCAN_STANDARD_ID;
	pTxHeader.MessageMarker = 0;
	pTxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	pTxHeader.TxFrameType = msg->flags;

	pTxHeader.Identifier = msg->id & CANID_MASK;

    switch (msg->len) {
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
    if (data == NULL) {
        printf("Keine Daten zum Drucken.\n");
    }

    // Daten als unsigned char* interpretieren
    unsigned char *data_ptr = (unsigned char *)data;

    // Puffer für die formatierte Ausgabe
    char data_str[256]; // Angenommene Puffergröße, die groß genug ist
    int offset = 0;

    // Formatieren der Daten in den String
    offset += sprintf(data_str + offset, "  ID: 0x%X", msg->id);
    offset += sprintf(data_str + offset, "  Length: %d", msg->len);
    offset += sprintf(data_str + offset, "  Flags: 0x%X\r\n", msg->flags);

    offset += sprintf(data_str + offset, "  Data: [ ");
    for (unsigned int i = 0; i < msg->len; ++i) {
        offset += sprintf(data_str + offset, "%02X ", data_ptr[i]); // Ausgabe der Daten im Hex-Format
    }
    sprintf(data_str + offset, "]"); // Füge das abschließende Newline hinzu


//	trace("sending CAN-Frame: %s", data_str);

//    tx_hdr.TxFrameType = (buffer->ident & FLAG_RTR) ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
//    tx_hdr.BitRateSwitch = FDCAN_BRS_OFF;
//    tx_hdr.MessageMarker = 0;
//    tx_hdr.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
//    tx_hdr.TxEventFifoControl = FDCAN_NO_TX_EVENTS;

	int e =  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &pTxHeader, data);
	if(e != HAL_OK){
	    RTC_DateTypeDef gDate;
	    RTC_TimeTypeDef gTime;
	    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
	    //Display time Format: hh:mm:ss
	    trace("[ %02d:%02d:%02d ] failed sending CAN-Frame",gTime.Hours, gTime.Minutes, gTime.Seconds);
	}
}
static int
on_can_send(const struct can_msg *msg, void *data)
{
	(void)data;
	return can_send(msg, 1) == 1 ? 0 : -1;
}

static void
on_nmt_cs(co_nmt_t *nmt, co_unsigned8_t cs, void *data)
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

static void
on_time(co_time_t *time, const struct timespec *tp, void *data)
{
	(void)time;
	(void)data;

	// Update the wall clock, _not_ the monotonic clock used by the CAN
	// network.
	clock_settime(CLOCK_REALTIME, tp);
}

static co_unsigned32_t
on_dn_2000_00(co_sub_t *sub, struct co_sdo_req *req, void *data)
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
//	assert(type == CO_DEFTYPE_UNSIGNED32);

	// This callback is invoked for every SDO CAN frame. Unless the value is
	// too large to be held in memory (for example, during a firmware
	// update), it is more convenient to wait until the entire value is
	// received. This is what the following call is designed to do. It
	// returns -1 with ac == 0 if more CAN frames are pending.
	union co_val val;
	if (co_sdo_req_dn_val(req, type, &val, &ac) == -1)
		return ac;

	// Check if the value is valid.
//	if (val.u32 != 42) {
//		ac = CO_SDO_AC_PARAM;
	//	goto error;
//	}

	// TODO: Do something with val.u32.

	// Write the temporary value to the local object dictionary.
	co_sub_dn(sub, &val);
error:
	// Finalize the temporary value. This is only necessary to cleanup array
	// values, but it is a good practice to always include it.
	co_val_fini(type, &val);
	return ac;
}

static co_unsigned32_t
on_up_2001_00(const co_sub_t *sub, struct co_sdo_req *req, void *data)
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

/* USER CODE END 4 */

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	trace("ERROR:Program enterred Error_Handler!");
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
