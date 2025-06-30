/*
 * uart_dma.h
 *
 *  Created on: Jun 30, 2025
 *      Author: WangblowsMicroshaft
 */

#ifndef UART_DMA_H_
#define UART_DMA_H_

#define TCD0_ADDRESS                             0x40210000
#define CH0_INT 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x8UL ) ) ) )
#define TCD0_CITER 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x36UL ) ) ) )
#define TCD0_BITER 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x3EUL ) ) ) )
#define TCD0_SADDR 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x20UL ) ) ) )
#define TCD0_SOFF 		                      ( *( ( ( volatile int16_t * ) ( TCD0_ADDRESS + 0x24UL ) ) ) )
#define TCD0_ATTR 		                      ( *( ( ( volatile uint16_t * ) ( TCD0_ADDRESS + 0x26UL ) ) ) )
#define TCD0_NBYTES 		                  ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x28UL ) ) ) )
#define TCD0_SLAST 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x2CUL ) ) ) )
#define TCD0_DADDR 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x30UL ) ) ) )
#define TCD0_DOFF 		                      ( *( ( ( volatile int16_t * ) ( TCD0_ADDRESS + 0x34UL ) ) ) )
#define TCD0_DLAST 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x38UL ) ) ) )
#define TCD0_CSR 		                      ( *( ( ( volatile uint32_t * ) ( TCD0_ADDRESS + 0x3CUL ) ) ) )

void DMA_UART_setup(void *buffer, uint32_t uart_addr);
void DMA_start(void);

#endif /* UART_DMA_H_ */
