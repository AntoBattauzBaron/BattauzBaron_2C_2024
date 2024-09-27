/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 27/09/2024 | Document creation		                         |
 *
 * @author Battauz Baron Antonella (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @brief Período en microsegundos para la activación del temporizador. Periodo de muestreo
 */
#define CONFIG_REFRESH 2000

/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea para medir la distancia.
 */
TaskHandle_t muestrear_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(muestrear_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}

static void muestrearTask(void *pvParameter){

	uint16_t voltaje = 0;

	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &voltaje);
		UartSendString(UART_PC, (const char*)UartItoa(voltaje,10));
		UartSendString(UART_PC, " \r\n");
	}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	timer_config_t timer = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer);

	analog_input_config_t  analog = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec =NULL
	};
	AnalogInputInit(&analog);
	xTaskCreate(&muestrearTask, "Muestrear", 2048, NULL, 5, &muestrear_task_handle);
	TimerStart(timer.timer);
	serial_config_t puertoSerie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL

	};
	UartInit(&puertoSerie);
}
/*==================[end of file]============================================*/