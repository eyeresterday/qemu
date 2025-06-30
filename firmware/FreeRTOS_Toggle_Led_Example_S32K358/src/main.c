/******************************************************************************
 *                                                                            *
 *   FreeRTOS sample application for RTD on S32 platforms                     *
 *                                                                            *
 *   Copyright 2023 NXP                                                       *
 *                                                                            *
 ******************************************************************************/

/* Including necessary configuration files. */
#include "Clock_Ip.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "nvic.h"
#include "uart.h"
#include "uart_dma.h"

#define main_TASK_PRIORITY                ( tskIDLE_PRIORITY + 2 )

SemaphoreHandle_t sem_handle;
volatile BaseType_t testResult = 0x33U;
char buffer[17] __attribute__ ((aligned (16))) = {0};

void DMA_handler(void)
{
	CH0_INT = 1;
	xSemaphoreGiveFromISR(sem_handle, NULL);
}


void LPUART_handler(void)
{
	DMA_start();
}

void LoggerTask( void *pvParameters )
{
    (void)pvParameters;

    for( ;; )
    {
    	xSemaphoreTake(sem_handle, portMAX_DELAY);
        UART_printf("DMA transfer to circular buffer completed successfully.\n");
    }
}

void PrinterTask( void *pvParameters )
{
    (void)pvParameters;

    for( ;; )
    {
    	UART_printf("Current contents of the buffer: ");
    	UART_printf(buffer);
    	UART_printf("\n");
        /* Not very exciting - just delay... */
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


/**
* @brief        Main function of the example
* @details      Initializes the used drivers and uses 1 binary Semaphore and
*               2 tasks to toggle a LED.
*/
int main(void)
{
    /* Initialize Clock */
    //Clock_Ip_StatusType Status_Init_Clock = CLOCK_IP_ERROR;
    //Status_Init_Clock = Clock_Ip_Init(Clock_Ip_aClockConfig);

    //if (Status_Init_Clock != CLOCK_IP_SUCCESS)
    //{
    //    while(1); /* Error during initialization. */
    //}

    /* Initialize all pins using the Port driver */
    //Siul2_Port_Ip_PortStatusType Status_Init_Port = SIUL2_PORT_ERROR;
    //Status_Init_Port = Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    //if(Status_Init_Port != SIUL2_PORT_SUCCESS)
    //{
    //    while(1); /* Error during initialization. */
    //}
	UART_init();
	DMA_UART_setup(buffer, UART0_ADDRESS + 0x1CUL);

    vSemaphoreCreateBinary(sem_handle);
    xSemaphoreTake(sem_handle, portMAX_DELAY);
    xTaskCreate( LoggerTask   , ( const char * const ) "LogTask", configMINIMAL_STACK_SIZE, (void*)0, main_TASK_PRIORITY, NULL );
    xTaskCreate( PrinterTask, ( const char * const ) "PrntTask" , configMINIMAL_STACK_SIZE, (void*)0, main_TASK_PRIORITY + 1, NULL );
    vTaskStartScheduler();


    for( ;; );

    return 0;
}

/** @} */
