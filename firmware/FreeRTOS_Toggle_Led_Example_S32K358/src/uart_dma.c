/*
 * uart_dma.c
 *
 *  Created on: Jun 30, 2025
 *      Author: WangblowsMicroshaft
 */

#include "nvic.h"
#include "uart_dma.h"

void DMA_UART_setup(void *buffer, uint32_t uart_addr) {
	TCD0_CITER = 1;
	TCD0_BITER = 1;
	TCD0_NBYTES = 4;
	TCD0_SADDR = uart_addr;
	TCD0_SOFF = 0;
	TCD0_SLAST = 0;
	TCD0_DADDR = buffer;
	TCD0_DOFF = 1;
	TCD0_DLAST = 0;
	TCD0_ATTR = (4 << 3); //16 bit circular destination buffer
	TCD0_CSR |= 1 << 1;
	NVIC_EnableIRQ(20);
	NVIC_SetPriority(20, 85);
}

void DMA_start(void) {
	TCD0_CSR |= 1;
}

