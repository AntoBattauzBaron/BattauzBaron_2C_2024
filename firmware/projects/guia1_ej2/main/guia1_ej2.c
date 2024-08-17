/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 *  Este programa hace titilar los leds 1 y 2 al mantener presionada las teclas 1 y 2 correspondientemente.
 *  También hace titilar el led 3 al presionar simultáneamente las teclas 1 y 2.
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
 * | 02/08/2024 | Document creation		                         |
 *
 * @author Battauz Barón Antonella (antobattauzbaron.abb@gmail.com)
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();

    while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas)
		{
    		case SWITCH_1:
    			LedToggle(LED_1); //conmuta el led 1
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2); //conmuta el led 2
			break;
			case (SWITCH_1 | SWITCH_2):
				LedToggle(LED_3); //conmuta el led 3
    		break;
    	}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
}
}
/*==================[end of file]============================================*/


/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

