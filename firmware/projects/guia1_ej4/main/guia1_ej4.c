/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa convierte un dato de 32 bits en binario natural a BCD
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
 * | 09/08/2024 | Document creation		                         |
 *
 * @author Antonella Battauz Barón (antobattauzbaron.abb@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number) 
{
	for(int i=0; i<digits; i++)
	{
		bcd_number[digits-1-i]=data%10; //Comienza a almacenar en la última posición del arreglo.
		data=data/10;
	}
	return 0;
}

/*==================[external functions definition]==========================*/

void app_main(void){

		uint32_t bin_de_32bits=123456789; // dato de 32 bits
		uint8_t cant_dig_BCD=9; //cantidad de dígitos de salida
		uint8_t array [cant_dig_BCD]; //arreglo donde se almacenan los datos de salida
		convertToBcdArray(bin_de_32bits, cant_dig_BCD, array);
}
/*==================[end of file]============================================*/