/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este codigo permite medir distancias mediante ultrasonido. Se puede detener 
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
 * | 13/09/2024 | Document creation		                         |
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

/*==================[macros and definitions]=================================*/

#define CONFIG_PERIOD_LEDS 1000
#define CONFIG_REFRESH 1000000
bool activar=1;
bool hold=0;
uint8_t teclas=0;

/*==================[internal data definition]===============================*/

TaskHandle_t medirDistancia_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param)
{
	vTaskNotifyGiveFromISR(medirDistancia_task_handle, pdFALSE);
}
static void medirDistanciaTask(void *pvParameter){

	uint16_t distancia;

    while(1){
	
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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
void ActivarDesactivar()
{
	activar =! activar;
}
void mantener()
{
	hold =! hold;
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
}
/*==================[end of file]============================================*/