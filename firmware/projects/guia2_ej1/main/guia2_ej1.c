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
 * | 06/09/2024 | Document creation		                         |
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

bool activar=1;
bool hold=0;
uint8_t teclas=0;
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
TaskHandle_t medirDistancia_task_handle = NULL;
TaskHandle_t leerTeclado_task_handle = NULL;

#define CONFIG_PERIOD_LEDS 1000
#define CONFIG_PERIOD_TECLADO 200
#define CONFIG_PERIOD 10
/*==================[internal functions declaration]=========================*/
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
		vTaskDelay(CONFIG_PERIOD_LEDS / portTICK_PERIOD_MS);
	}
}
static void leerTecladoTask(void *pvParameter){

	while(1)
	{
	teclas  = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			activar=!activar;
			vTaskDelay(CONFIG_PERIOD_TECLADO / portTICK_PERIOD_MS);
			break;
		case SWITCH_2:
			hold=!hold;
			vTaskDelay(CONFIG_PERIOD_TECLADO / portTICK_PERIOD_MS);
			break;
		}

		vTaskDelay(CONFIG_PERIOD / portTICK_PERIOD_MS);
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