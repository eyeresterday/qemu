/*
 * uart.c
 *
 *  Created on: Jun 30, 2025
 *      Author: Mateus Ferreira Varela Cancado
 */
#include "uart.h"
#include "nvic.h"


void UART_printf(const char *s) {
    while(*s != '\0') {
        UART0_DATA = (unsigned int)(*s);
        s++;
    }
}

void UART_echo(void) {
    UART0_DATA = UART0_DATA;
}

void UART_init( void )
{
    UART0_BAUDDIV = 16;
    UART0_CTRL = 1 | (1 << 18) | (1 << 21); // Enable interrupts
    //Set FIFO Size
    UART0_FIFO |= (1 << 3) | (001);
	NVIC_EnableIRQ(141);
	NVIC_SetPriority(141, 85);
}



