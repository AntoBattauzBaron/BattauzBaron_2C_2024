/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa permite visualizar un numero de 3 dígitos en un display
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |     EDU-ESP    |   PERIFERICO 	|
 * |:--------------:|:--------------|
 * | 	GPIO_20	 	| 		D1		|
 * | 	GPIO_21	 	| 		D2		|
 * | 	GPIO_22	 	| 		D3		|
 * | 	GPIO_23	 	| 		D4		|
 * | 	GPIO_19	 	| 	  SEL_1	 	|
 * | 	GPIO_18	 	| 	  SEL_2	 	|
 * | 	GPIO_9	 	| 	  SEL_3	 	|
 * | 	 +5V	 	| 	   +5V	 	|
 * | 	 GND	 	| 	   GND	 	|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/08/2024 | Document creation		                         |
 *
 * @author Battauz Barón Antonella (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <gpio_mcu.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief Informacion de los pines
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Convierte un numero decimal en BCD
 * 
 * @param data dato a convertir en BCD
 * @param digits cantidad de digitos de salida
 * @param bcd_number puntero al vector que almacena el dato en formato BCD
 * 
 * @return 
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number) 
{
	for(int i=0; i<digits; i++)
	{
		bcd_number[digits-1-i]=data%10;
		data=data/10;
	}
	return 0;
}
/**
 * @brief Configura el estado de los pines segun un numero en BCD
 * 
 * @param vector vector con los pines
 * @param digito digito BCD
 * @return 
 * 
 */
void estado_pines (gpioConf_t vector[], uint8_t digito)
{
	int estado;
	for (int i = 0; i < 4; i++)
	{
		estado=digito>>i;
		GPIOState(vector[i].pin, estado&1);
	}
}
/**
 * @brief Configura el estado de los GPIO según un numero para mostrarlo en un display
 * 
 * @param numero valor que se quiere mostrar en display
 * @param cant_digitos cantidad de dígitos que tiene el numero que se quiere mostrar en el display
 * @param vector vector con los pines que establecen el numero (en BCD) a mostrar
 * @param habilitaciones vector con los pines de las habilitaciones para enviar un pulso 
 * @return 
 * 
 */
void mostar_en_display(uint32_t numero, uint8_t cant_digitos, gpioConf_t vector[], gpioConf_t habilitaciones[])
{
	uint8_t valor[cant_digitos];
	convertToBcdArray(numero, cant_digitos, valor);

	for(int i=0; i<3; i++)
	{
		estado_pines(vector, valor[i]);
		GPIOState(habilitaciones[i].pin, 1);
		GPIOState(habilitaciones[i].pin, 0);
	}
}

/*==================[external functions definition]==========================*/

void app_main(void){

	gpioConf_t vector [4];
	gpioConf_t habilitaciones[3];
	uint32_t digito=343; //Dígito a mostrar
	uint8_t cantidad_digitos=3;

	vector[0].pin=GPIO_20;
	vector[1].pin=GPIO_21;
	vector[2].pin=GPIO_22;
	vector[3].pin=GPIO_23;

	habilitaciones[0].pin=GPIO_19;
	habilitaciones[1].pin=GPIO_18;
	habilitaciones[2].pin=GPIO_9;

	for(int i=0; i<4; i++)
	{
		vector[i].dir=GPIO_OUTPUT;
		GPIOInit(vector[i].pin, vector[i].dir);
	}

	for(int i=0; i<3;i++)
	{
		habilitaciones[i].dir=GPIO_OUTPUT;
		GPIOInit(habilitaciones[i].pin, habilitaciones[i].dir);
	}
	mostar_en_display(digito, cantidad_digitos, vector, habilitaciones);

}
/*==================[end of file]============================================*/