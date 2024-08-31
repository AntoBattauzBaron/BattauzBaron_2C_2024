/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa cambia de estado de los GPIO según un número BCD ingresado
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
 * | 16/08/2024 | Document creation		                         |
 *
 * @author Battauz Barón Antonella (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/

void estado_pines (gpioConf_t vector[], uint8_t digito) //Esta función setea los GPIO según el valor del dígito en BCD
{
	uint8_t estado;
	for (int i = 0; i < 4; i++)
	{
		estado=digito>>i;
		GPIOState(vector[i].pin, estado&1);
	}
}

/*==================[external functions definition]==========================*/

void app_main(void){
	gpioConf_t vector [4]; //Vector de struct 
	uint8_t digito=2; //Este valor puede cambiarse del 0 al 9 

	vector[0].pin=GPIO_20;
	vector[1].pin=GPIO_21;
	vector[2].pin=GPIO_22;
	vector[3].pin=GPIO_23;

	for(int i=0; i<4; i++)
	{
		vector[i].dir=GPIO_OUTPUT; //se establecen los pines como salida
		GPIOInit(vector[i].pin, vector[i].dir); //Se inicializan los pines
	}

	estado_pines(vector, digito);
}
/*==================[end of file]============================================*/