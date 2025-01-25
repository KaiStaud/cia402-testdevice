/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdlib.h>
#include <stdio.h>
#include <lely/co/nmt.h>
#include <lely/co/sdev.h>
#include <lely/co/sdo.h>
#include <lely/co/time.h>
#include <lely/co/co.h>
#include <lely/bsp/can.h>

#include <lcd/lcd.h>
#include <ugui/ugui.h>
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

FDCAN_HandleTypeDef hfdcan1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t ubKeyNumber = 0x0;
FDCAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
FDCAN_TxHeaderTypeDef TxHeader;
uint8_t TxData[8]={'F','D','C','A','N',1,2,3};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */
static int on_can_send(const struct can_msg *msg, void *data);
static void on_nmt_cs(co_nmt_t *nmt, co_unsigned8_t cs, void *data);
static void on_time(co_time_t *time, const struct timespec *tp, void *data);
static co_unsigned32_t on_dn_2000_00(co_sub_t *sub, struct co_sdo_req *req,void *data);
static co_unsigned32_t on_up_2001_00(const co_sub_t *sub,struct co_sdo_req *req, void *data);
static co_unsigned32_t on_dn_6040_00(co_sub_t *sub, struct co_sdo_req *req,void *data);
static co_unsigned32_t on_up_6041_00(const co_sub_t *sub,struct co_sdo_req *req, void *data);
extern const struct co_sdev lpc17xx_sdev;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
struct timespec now = { 0, 0 };
co_dev_t *dev;
co_nmt_t *nmt;
can_net_t *net;

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
  MX_FDCAN1_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
 //HAL_TIM_Base_Start_IT(&htim7);
  can_init(125);;
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
	// Initialize the CAN network interface.
	net = can_net_create();
	assert(net);
	can_net_set_send_func(net, &on_can_send, NULL);

	// Initialize the CAN network clock. 
//	struct timespec now = { 0, 0 };
	clock_gettime(1, &now);
	can_net_set_time(net, &now);

	// Create a dynamic object dictionary from the static object dictionary.
	dev = co_dev_create_from_sdev(&lpc17xx_sdev);
	assert(dev);

	// Create the CANopen NMT service.
	nmt = co_nmt_create(net, dev);
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


	// Set the download (SDO write) indication function for sub-object
	// 2000:01.
	co_sub_set_dn_ind(co_dev_find_sub(dev, 0x6040, 0x00), &on_dn_6040_00,
			NULL);

	// Set the upload (SDO read) indication function for sub-object 2001:01.
	co_sub_set_up_ind(co_dev_find_sub(dev, 0x6041, 0x00), &on_up_6041_00,
			NULL);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_Delay(1000);
	// Update the CAN network clock.
	clock_gettime(1, &now);

	  //struct tm *tm_info;
    //tm_info = localtime(&now.tv_sec);
    RTC_DateTypeDef gDate;
    RTC_TimeTypeDef gTime;
    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
  	can_net_set_time(net, &now);

	// Process any received CAN frames.
	struct can_msg msg;
	int n_frames = can_recv(&msg, 1);
		if(n_frames !=0)
		{
		    // Puffer für die formatierte Ausgabe
		    char data_str[256]; // Angenommene Puffergröße, die groß genug ist
		    int offset = 0;

		    // Formatieren der Daten in den String
		    offset += sprintf(data_str + offset, "  ID: 0x%X", (unsigned int)msg.id);
		    offset += sprintf(data_str + offset, "  Length: %d", msg.len);
		    offset += sprintf(data_str + offset, "  Flags: 0x%X\r\n", msg.flags);

		    offset += sprintf(data_str + offset, "  Data: [ ");
		    for (unsigned int i = 0; i < msg.len; ++i) {
		        offset += sprintf(data_str + offset, "%02X ", msg.data[i]); // Ausgabe der Daten im Hex-Format
		    }
		    sprintf(data_str + offset, "]"); // Füge das abschließende Newline hinzu
//			trace("Received frame from %s ",data_str);
			if(can_net_recv(net, &msg) == -1)
			{
//				trace("can net receive -1");
			}
		}
   
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 170;
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
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_DCX_Pin|LCD_RST_Pin|LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DCX_Pin LCD_RST_Pin LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_DCX_Pin|LCD_RST_Pin|LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static int on_can_send(const struct can_msg *msg, void *data)
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



static co_unsigned32_t on_dn_6040_00(co_sub_t *sub, struct co_sdo_req *req, void *data)
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

static co_unsigned32_t on_up_6041_00(const co_sub_t *sub, struct co_sdo_req *req, void *data)
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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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
