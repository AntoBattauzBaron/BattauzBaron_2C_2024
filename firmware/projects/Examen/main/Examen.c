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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "uart_mcu.h"
#include <gpio_mcu.h>
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/**
 * @brief Período en microsegundos para la activación del temporizador.
 */
#define CONFIG_REFRESH 500000
/**
 * @brief Período en microsegundos para la activación del temporizador.
 */
#define CONFIG_REFRESH_ACELEROMETRO 10000
#define FRECUENCIA_PRECAUCION 1000
#define FRECUENCIA_PELIGRO 500
#define ORDENADA_AL_ORIGEN_ACELERACION 1650
#define PENDIENTE_ACELERACION 300
#define UMBRAL_ACCIDENTE 4
bool peligro = false;
bool precaución = false;
bool accidente = false;
	serial_config_t puertoSerie = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief Handle de la tarea para medir la distancia.
 */
TaskHandle_t medirDistancia_task_handle = NULL;

/**
 * @brief Handle de la tarea para medir la distancia.
 */
TaskHandle_t medirAceleracion_task_handle = NULL;

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(medirDistancia_task_handle, pdFALSE); /* Envía una notificación a la tarea */
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerB(void* param)
{
	vTaskNotifyGiveFromISR(medirAceleracion_task_handle, pdFALSE); /* Envía una notificación a la tarea */
}

void control_buzzer()
{
	if(peligro)
	{
		GPIOState(GPIO_20, 1);
		vTaskDelay(FRECUENCIA_PELIGRO / portTICK_PERIOD_MS);
		GPIOState(GPIO_20, 0);
	}

	else if(precaución)
	{
		GPIOState(GPIO_20, 1);
		vTaskDelay(FRECUENCIA_PRECAUCION / portTICK_PERIOD_MS);
		GPIOState(GPIO_20, 0);		
	}

	else
	{
		GPIOState(GPIO_20, 0);
	}
}
void avisar_accidente()
{
	if(accidente)
	{
		UartSendString(UART_CONNECTOR, "Caída detectada \r\n");
	}
}
static void medirDistanciaTask(void *pvParameter){

	uint16_t distancia = 0;

	while(1){
	
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	distancia = HcSr04ReadDistanceInCentimeters();

		if(distancia>500)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}

		else if(distancia>300 && distancia<500)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
			precaución = true;
			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca \r\n");
		}

		else if(distancia<300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
			peligro = true;
			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca \r\n");
		}

		control_buzzer();
	}
}
static void medirAceleracionTask(void *pvParameter){

	uint16_t aceleracion_en_x = 0;
	uint16_t aceleracion_en_y = 0;
	uint16_t aceleracion_en_z = 0;

	uint16_t voltaje_en_x = 0;
	uint16_t voltaje_en_y = 0;
	uint16_t voltaje_en_z = 0;

	analog_input_config_t  aceleracion_x = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec =NULL
	};
	AnalogInputInit(&aceleracion_x);

	analog_input_config_t  aceleracion_y = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec =NULL
	};
	AnalogInputInit(&aceleracion_y);

	analog_input_config_t  aceleracion_z = {
		.input = CH3,
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL,
		.sample_frec =NULL
	};
	AnalogInputInit(&aceleracion_z);

	while(1){
	
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		AnalogInputReadSingle(CH1, &voltaje_en_x);
		AnalogInputReadSingle(CH2, &voltaje_en_y);
		AnalogInputReadSingle(CH3, &voltaje_en_z);

		aceleracion_en_x = (voltaje_en_x - ORDENADA_AL_ORIGEN_ACELERACION) / PENDIENTE_ACELERACION;
		aceleracion_en_y = (voltaje_en_y - ORDENADA_AL_ORIGEN_ACELERACION) / PENDIENTE_ACELERACION;
		aceleracion_en_z = (voltaje_en_z - ORDENADA_AL_ORIGEN_ACELERACION) / PENDIENTE_ACELERACION;

		if((aceleracion_en_x + aceleracion_en_y + aceleracion_en_z) >= UMBRAL_ACCIDENTE)
		{
			accidente = true;
		}
		
		else
		{
			accidente = false;
		}
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);

	timer_config_t timer_A = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

	timer_config_t timer_B = {
        .timer = TIMER_B,
        .period = CONFIG_REFRESH_ACELEROMETRO,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
	TimerInit(&timer_B);

	TimerInit(&timer_A);
    xTaskCreate(&medirDistanciaTask, "MedirDistancia", 512, NULL, 5, &medirDistancia_task_handle);
	xTaskCreate(&medirAceleracionTask, "MedirAceleracion", 512, NULL, 5, &medirAceleracion_task_handle);
	TimerStart(timer_A.timer);
	TimerStart(timer_B.timer);
	GPIOInit(GPIO_20, GPIO_OUTPUT);
	UartInit(&puertoSerie);


}
/*==================[end of file]============================================*/