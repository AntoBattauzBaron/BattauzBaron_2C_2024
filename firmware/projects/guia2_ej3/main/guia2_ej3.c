/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este codigo permite medir distancias mediante ultrasonido. Se puede detener la medición y mantener el valor en pantalla
 * con interrupciones. Además el puerto serie permite visualizar las mediciones en la computadora
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |  EDU-CIAA-NXP |
 * |:--------------:|:--------------|
 * | 	 ECHO	 	| 	 GPIO_3		|
 * | 	TRIGGER	 	| 	 GPIO_2		|
 * | 	  +5V	 	| 	  +5V		|
 * | 	  GND	 	| 	  GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/09/2024 | Document creation		                         |
 *
 * @author Battauz Baron Antonella (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/**
 * @brief Período en microsegundos para la activación del temporizador.
 */
#define CONFIG_REFRESH 1000000
/**
 * @brief Indicador para activar o desactivar la medición, inicia activado
 */
bool activar=1;
/**
 * @brief Indicador para mantener o no la medición, inicia sin mantener
 */
bool hold=0;
/**
 * @brief Estado en la tecla medida
 */
uint8_t teclas=0;


/*==================[internal data definition]===============================*/
/**
 * @brief Handle de la tarea para medir la distancia.
 */
TaskHandle_t medirDistancia_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(medirDistancia_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada */
}
/**
 * @brief Tarea que mide la distancia usando un sensor ultrasónico y controla los LEDs en base a dicha distancia.
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
static void medirDistanciaTask(void *pvParameter){

	uint16_t distancia=0;

    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	if(activar == true)
	{
		distancia = HcSr04ReadDistanceInCentimeters();

		if(distancia<10)
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		else if(distancia>10 && distancia<20)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		else if(distancia>20 && distancia<30)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
		}
		else if(distancia>30)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}

		if(hold == false)
		{
			LcdItsE0803Write(distancia);
		}
		
		UartSendString(UART_PC, (const char*)UartItoa(distancia,10));
		UartSendString(UART_PC, " cm\r\n");
	}
	else if(activar==false)
	{
		LcdItsE0803Off();
		LedsOffAll();
	}
		
	}
	
}
/**
 * @brief Función que alterna el estado de activación de la medición de distancia.
 */
void ActivarDesactivar()
{
	activar =! activar;
}
/**
 * @brief Función que alterna el estado de mantenimiento de la visualización en el display.
 */
void mantener()
{
	hold =! hold;
}
/**
 * @brief Función que lee el teclado de la PC por el puerto serie y llama a cambiar el estado de activar y hold
 */
void atenderTecladoPC()
{
	uint8_t tecladoPC=0;
	UartReadByte(UART_PC, &tecladoPC);

	switch (tecladoPC)
	{
	case 'O':
		ActivarDesactivar();
		break;
	
	case 'H':
		mantener();
		break;

	default:
		break;
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	LcdItsE0803Init();
	SwitchesInit();
	timer_config_t timer = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
	TimerInit(&timer);
    xTaskCreate(&medirDistanciaTask, "MedirDistancia", 512, NULL, 5, &medirDistancia_task_handle);
	TimerStart(timer.timer);
	SwitchActivInt(SWITCH_1, &ActivarDesactivar, NULL);
	SwitchActivInt(SWITCH_2, &mantener, NULL);
	serial_config_t puertoSerie = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = atenderTecladoPC,
		.param_p = NULL
	};
	UartInit(&puertoSerie);
}
/*==================[end of file]============================================*/