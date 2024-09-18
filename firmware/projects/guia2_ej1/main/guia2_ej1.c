/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este codigo permite medir distancias mediante ultrasonido. Se puede detener la medición y mantener el valor en pantalla
 *
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
 * | 06/09/2024 | Document creation		                         |
 *
 * @author Battauz Barón Antonella (antobattauzbaron.abb@gmail.com)
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

/*==================[macros and definitions]=================================*/
/**
 * @brief Período en milisegundos para la tarea que mide la distancia
 */
#define CONFIG_PERIOD_LECTURA 1000
/**
 * @brief Período en milisegundos para la tarea de control si se presionó una tecla.
 */
#define CONFIG_PERIOD_PRESIONADO 200
/**
 * @brief Período en milisegundos para la tarea de control del teclado
 */
#define CONFIG_PERIOD_TECLADO 10
/**
 * @brief Indicador para activar o desactivar la medición
 */
bool activar=1;
/**
 * @brief Indicador para mantener o no la medición
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
/**
 * @brief Handle de la tarea para leer el teclado.
 */
TaskHandle_t leerTeclado_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Tarea que mide la distancia usando un sensor ultrasónico y controla los LEDs en base a dicha distancia.
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
static void medirDistanciaTask(void *pvParameter){

	uint16_t distancia;

    while(1){
		
	if(activar)
	{
		distancia=HcSr04ReadDistanceInCentimeters();
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

		if(!hold)
		{
			LcdItsE0803Write(distancia);
		}
	}
	else if(activar==false)
	{
		LcdItsE0803Off();
		LedsOffAll();
	}
		vTaskDelay(CONFIG_PERIOD_LECTURA / portTICK_PERIOD_MS);
	}
}
/**
 * @brief Tarea que lee el estado de las teclas y activa/desactiva la medición o la visualización en el display
 *
 * @param pvParameter parámetro de uso interno del sistema operativo
 */
static void leerTecladoTask(void *pvParameter){

	while(1)
	{
	teclas  = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			activar=!activar;
			vTaskDelay(CONFIG_PERIOD_PRESIONADO / portTICK_PERIOD_MS);
			break;
		case SWITCH_2:
			hold=!hold;
			vTaskDelay(CONFIG_PERIOD_PRESIONADO / portTICK_PERIOD_MS);
			break;
		}

		vTaskDelay(CONFIG_PERIOD_TECLADO / portTICK_PERIOD_MS);
	}

}
/*==================[external functions definition]==========================*/
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	LcdItsE0803Init();
	SwitchesInit();
    xTaskCreate(&medirDistanciaTask, "MedirDistancia", 512, NULL, 5, &medirDistancia_task_handle);
	xTaskCreate(&leerTecladoTask, "LeerTeclado", 512, NULL, 5, &leerTeclado_task_handle);
}
/*==================[end of file]============================================*/