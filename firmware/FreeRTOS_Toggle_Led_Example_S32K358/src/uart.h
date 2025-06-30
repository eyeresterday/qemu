/*
 * uart.h
 *
 *  Created on: Jun 30, 2025
 *      Author: Mateus Ferreira Varela Cancado
 */

#ifndef UART_H_
#define UART_H_



#define UART0_ADDRESS                        0x40328000

#define UART0_DATA                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0x1CUL ) ) ) )
#define UART0_STATE                           ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 14UL ) ) ) )
#define UART0_CTRL                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 24UL ) ) ) )
#define UART0_BAUDDIV                         ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 16UL ) ) ) )
#define UART0_FIFO 		                      ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 40UL ) ) ) )

void UART_printf(const char *s);
void UART_echo(void);
void UART_init( void );
#endif /* UART_H_ */
