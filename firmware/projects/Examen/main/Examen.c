/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este código resuelve el examen parcial de Electrónica Programable. Es un detector de eventos peligrosos para ciclistas.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |     ESP32      |     PERIFERICO    |
 * |:--------------:|:------------------|
 * | 	GPIO_20	 	|  CH_x_Aceleromet  |
 * | 	GPIO_21	 	|  CH_y_Aceleromet  |
 * | 	GPIO_22	 	|  CH_z_Aceleromet  |
 * | 	GPIO_23	 	|    Señal buzzer   |
 * | 	 ECHO	 	| 	   GPIO_3	    |
 * | 	TRIGGER	 	| 	   GPIO_2	    |
 * | 	  +5V	 	| 	     +5V		|
 * | 	  GND	 	| 	     GND        |
 * |    GPIO_18	 	| 	     RX		    |
 * | 	GPIO_19	    | 	     TX         |

 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Antonella Battauz Baron (antobattauzbaron.abb@gmail.com)
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
 * @brief Período en microsegundos para la activación del temporizador para medir distancia.
 */
#define CONFIG_REFRESH_MEDIR 500000
/**
 * @brief Período en microsegundos para la activación del temporizador para medir aceleración.
 */
#define CONFIG_REFRESH_ACELEROMETRO 10000
/**
 * @brief Período en milisegundos del buzzer cuando avisa precaución.
 */
#define PERIODO_PRECAUCION 1000
/**
 * @brief Período en milisegundos del buzzer cuando avisa peligro.
 */
#define PERIODO_PELIGRO 500
/**
 * @brief Ordenada al origen de la función para el acelerómetro.
 */
#define ORDENADA_AL_ORIGEN_ACELERACION 1650
/**
 * @brief Pendiente de la función para el acelerómetro.
 */
#define PENDIENTE_ACELERACION 300
/**
 * @brief Umbral en G a partir del que se considera que ocurrió un accidente.
 */
#define UMBRAL_ACCIDENTE 4
/**
 * @brief Variable que indica si hay peligro o no (distancia < 3m)
 */
bool peligro = false;
/**
 * @brief Variable que indica si hay precaución o no o no (3 < distancia < 5)
 */
bool precaución = false;
/**
 * @brief puerto serie por el que se envía info al módulo bluetooth por el segundo puerto serie
 */
serial_config_t puertoSerie = {
	.port = UART_CONNECTOR,
	.baud_rate = 115200,
	.func_p = NULL,
	.param_p = NULL
};
/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea para medir la distancia.
 */
TaskHandle_t medirDistancia_task_handle = NULL;

/**
 * @brief Handle de la tarea para medir la aceleración.
 */
TaskHandle_t medirAceleracion_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(medirDistancia_task_handle, pdFALSE); /* Envía una notificación a la tarea */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param)
{
	vTaskNotifyGiveFromISR(medirAceleracion_task_handle, pdFALSE); /* Envía una notificación a la tarea */
}
/**
 * @brief Función que controla la activación y la frecuencia del buzzer
 */
void control_buzzer()
{
	if(peligro)
	{
		GPIOState(GPIO_23, 1);
		vTaskDelay(PERIODO_PELIGRO / portTICK_PERIOD_MS);
		GPIOState(GPIO_23, 0);
		vTaskDelay(PERIODO_PELIGRO / portTICK_PERIOD_MS);
	}

	else if(precaución)
	{
		GPIOState(GPIO_23, 1);
		vTaskDelay(PERIODO_PRECAUCION / portTICK_PERIOD_MS);
		GPIOState(GPIO_23, 0);	
		vTaskDelay(PERIODO_PRECAUCION / portTICK_PERIOD_MS);	
	}

	else
	{
		GPIOState(GPIO_23, 0);
	}
}
/**
 * @brief Tarea que mide la distancia por ultrasonido, controla los leds y avisa mediante bluetooth
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
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
				precaución = false;
				peligro = false;
			}

			else if(distancia>300 && distancia<500)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
				precaución = true;
				peligro = false;
				UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca \r\n");
			}

			else if(distancia<300)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
				peligro = true;
				precaución = false;
				UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca \r\n");
			}
			control_buzzer();
	}
}
/**
 * @brief Tarea que mide la señal del acelerómetro y calcula la aceleración
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
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
			UartSendString(UART_CONNECTOR, "Caída detectada \r\n");
		}
		
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);

	timer_config_t timer_A = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH_MEDIR,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer_A);
	TimerStart(timer_A.timer);

	timer_config_t timer_B = {
        .timer = TIMER_B,
        .period = CONFIG_REFRESH_ACELEROMETRO,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
	TimerInit(&timer_B);
	TimerStart(timer_B.timer);

    xTaskCreate(&medirDistanciaTask, "MedirDistancia", 512, NULL, 5, &medirDistancia_task_handle);
	xTaskCreate(&medirAceleracionTask, "MedirAceleracion", 512, NULL, 5, &medirAceleracion_task_handle);

	GPIOInit(GPIO_23, GPIO_OUTPUT);
	UartInit(&puertoSerie);

}
/*==================[end of file]============================================*/