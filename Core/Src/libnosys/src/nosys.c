#include "nosys.h"

static char rxbuff[32];
static char txbuff[128];

//RINGBUFF_T __libnosys_rxring;
//RINGBUFF_T __libnosys_txring;

void
__libnosys_init(void)
{
	/*
	RingBuffer_Init(&__libnosys_rxring, rxbuff, 1, sizeof(rxbuff));
	RingBuffer_Init(&__libnosys_txring, txbuff, 1, sizeof(txbuff));

	// Pin configuration UART0: select P0.2 as TXD0 and P0.3 as RXD0.
	Chip_IOCON_PinMux(LPC_IOCON, 0, 2, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 3, IOCON_MODE_INACT, IOCON_FUNC1);

	// Setup UART for 115200-8-N-1.
	Chip_UART_Init(LPC_UART0);
	Chip_UART_SetBaud(LPC_UART0, 115200);
	Chip_UART_ConfigData(LPC_UART0, UART_LCR_WLEN8 | UART_LCR_SBS_1BIT
			| UART_LCR_PARITY_DIS);
	Chip_UART_TXEnable(LPC_UART0);

	// Reset and enable FIFOs, FIFO trigger level 3 (14 chars).
	Chip_UART_SetupFIFOS(LPC_UART0, UART_FCR_FIFO_EN | UART_FCR_RX_RS
			| UART_FCR_TX_RS | UART_FCR_TRG_LEV3);

	// Enable receive data and line status interrupt.
	Chip_UART_IntEnable(LPC_UART0, UART_IER_RBRINT | UART_IER_RLSINT);

	NVIC_EnableIRQ(UART0_IRQn);
	*/
}

void
__libnosys_fini(void)
{
	/*
	NVIC_DisableIRQ(UART0_IRQn);
	Chip_UART_DeInit(LPC_UART0);
	*/
}

void
UART0_IRQHandler(void)
{
	/*
	Chip_UART_IRQRBHandler(LPC_UART0, &__libnosys_rxring,
			&__libnosys_txring);
	*/
}
