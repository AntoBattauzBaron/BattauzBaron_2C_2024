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
 * @author Battauz Baron Antonella (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#define CONFIG_BLINK_PERIOD 1000

enum MODO {ON, OFF, TOGGLE}; //Modos de operación, ON=0, OFF=1, TOGGLE=2

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
struct leds
{
    uint8_t mode;        //ON (1), OFF (2), TOGGLE (3)
	uint8_t n_led;       //indica el número de led a controlar (1,2 o 3)
	uint8_t n_ciclos;    //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds;

void control_leds (struct leds* my_leds){ //Función que controla los leds

	if(my_leds->mode==0) //Enciende el led especificado
	{
		if(my_leds->n_led==1)
		{
        	LedOn(LED_1);
		}
		else if (my_leds->n_led==2)
		{
			LedOn(LED_2);
		}
		else if (my_leds->n_led==3)
		{
			LedOn(LED_3);
		}
	}

	else if (my_leds->mode==1) //Apaga el led especificado
	{
		if(my_leds->n_led==1)
		{
        	LedOff(LED_1);
		}
		else if (my_leds->n_led==2)
		{
			LedOff(LED_2);
		}
		else if (my_leds->n_led==3)
		{
			LedOff(LED_3);
		}
	}
	else if(my_leds->mode==2) //Conmuta el led especificado la cantidad de ciclos requeridas
	{
		if(my_leds->n_led==1)
		{
			for(int i=0; i<my_leds->n_ciclos; i++)
			{
				LedOn(LED_1);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        		LedOff(LED_1);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		}

		if(my_leds->n_led==2)
		{
			for(int i=0; i<my_leds->n_ciclos; i++)
			{
				LedOn(LED_2);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        		LedOff(LED_2);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		}

		if(my_leds->n_led==3)
		{
			for(int i=0; i<my_leds->n_ciclos; i++)
			{
				LedOn(LED_3);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        		LedOff(LED_3);
        		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		}
	}
}

void app_main(void){
	struct leds *foco;
	foco=&my_leds;
	foco->mode = OFF;
	foco->n_led=2;
	foco->n_ciclos=5;
	LedsInit();
	control_leds(foco);
}
/*==================[end of file]============================================*/