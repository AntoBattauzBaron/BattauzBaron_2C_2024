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
 * @brief Período en microsegundos para la activación del temporizador A. Periodo de muestreo (1/500Hz)
 */
#define CONFIG_REFRESH 2000
/**
 * @brief Tamaño del arreglo con los datos del ECG
 */
#define BUFFER_SIZE 231
/**
 * @brief Período en microsegundos para la activación del temporizador B para los datos de ECG
 */
#define CONFIG_REFRESH_ECG 4329 
/**
 * @brief Posición en el arreglo que contiene los datos de ECG
 */
uint16_t cuentas_ECG = 0;

/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea para enviar los datos de ECG en analógico
 */
TaskHandle_t enviar_task_handle = NULL;
/**
 * @brief Handle de la tarea para muestrear los datos de la señal analógica
 */
TaskHandle_t muestrear_task_handle = NULL;
/**
 * @brief Arreglo con los datos de ECG
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(muestrear_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_1 */
}
/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param)
{
	vTaskNotifyGiveFromISR(enviar_task_handle, pdFALSE);
}
/**
 * @brief Tarea que muestrea los datos de una señal en el canal 1
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
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
/**
 * @brief Tarea que envía datos analógicos por el canal 0
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
static void enviarTask(void *pvParameter){

	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogOutputWrite(ecg[cuentas_ECG]);

		if(cuentas_ECG<(BUFFER_SIZE-2))
		{
			cuentas_ECG++;
		}
		else
		{
			cuentas_ECG=0;
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	timer_config_t timer_A = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_A);
		timer_config_t timer_B = {
        .timer = TIMER_B,
        .period = CONFIG_REFRESH_ECG,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
	TimerInit(&timer_B);

	analog_input_config_t  analog = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec =NULL
	};
	AnalogInputInit(&analog);
	AnalogOutputInit();
	xTaskCreate(&muestrearTask, "Muestrear", 2048, NULL, 5, &muestrear_task_handle);
	xTaskCreate(&enviarTask, "Leer", 2048, NULL, 5, &enviar_task_handle);
	TimerStart(timer_A.timer);
	TimerStart(timer_B.timer);
	serial_config_t puertoSerie = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&puertoSerie);
}
/*==================[end of file]============================================*/