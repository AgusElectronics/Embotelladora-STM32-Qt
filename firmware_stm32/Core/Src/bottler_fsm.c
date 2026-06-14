/*
 * bottler_fsm.c
 *
 *  Created on: 14 may 2026
 *      Author: agust
 */


#include "bottler_fsm.h"
#include "main.h"
#include "usbd_cdc_if.h" //Importamos la librería del USB
#include <stdio.h>       //Para poder usar sprintf()
#include <string.h>      //Para poder usar strlen()

extern volatile uint8_t presencia;
extern volatile uint8_t contador;
extern TIM_HandleTypeDef htim2;
volatile uint8_t estado = 0;
volatile float temp_actual = 25.0f;
volatile uint8_t estado_anterior = 99;

void MaquinaEstados(void){

		// Temporizador para no saturar el USB
		// Usamos 'static' para que la variable NO se borre de la memoria al salir de la función y recuerde el tiempo en la siguiente vuelta del bucle.

		// 1. ENVIAR CAMBIO DE ESTADO INSTANTÁNEO
		if(estado != estado_anterior){ //Cuando cambiamos de estado..
			char mensaje[20]; //Declaramos y definimos una cadena de caracteres vacia que se reseteará cada vez que el superloop llame a la maquina de estados.
			sprintf(mensaje, "%d\r\n", estado); //Escribimos nuestro estado por encima de los caracteres vacios de la cadena.
			CDC_Transmit_FS((uint8_t*)mensaje, strlen(mensaje)); //Transmitimos la cadena de texto a la interfaz.
			estado_anterior = estado; //Guardamos nuestro nuevo estado como estado anterior para poder comparar en la siguiente iteración.
		}

		// 2. EMERGENCIA POR TEMPERATURA
		if (estado != 4) { // PRIORIZAMOS EL ESTADO DE LA SETA
			if (temp_actual > 72.0f && estado != 5) { // Si estamos a más 72ºC y no estamos en el estado 5.
				estado = 5;               // Entramos en emergencia térmica
				conveyorStop(); // Paramos la cinta.
				valveClose(); // Cerramos la valvula de golpe.
				HAL_TIM_Base_Stop_IT(&htim2); // Paramos el temporizador 2 (congelamos el tiempo).
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); // LED Rojo ON.
			}
			else if (temp_actual <= 72.0f && estado == 5) { // Si estamos a menos de 72ºC y estamos en el estado 5.

				estado = 0; //Empezamos de nuevo
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // Apagamos LED Rojo

				// Siempre tenemos que volver a encender el metrónomo, incluso si estábamos en el Estado 0.
				HAL_TIM_Base_Start_IT(&htim2);
				conveyorForward(); // Si estábamos esperando, la cinta vuelve a girar.
			}
		}

		//He separado el bloque de emergencia 5 del resto para que no hubiesen conflictos entre el case4 y el case5.
		//Hay que tener en cuenta que la emergencia prioritaria es la de la seta. Aunque baje la temperatura si no hemos quitado la seta no podemos volver.

	switch (estado){ //NUESTRA MAQUINA DE ESTADOS

		case 0: // ESPERANDO BOTELLA
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); //Apagamos el led de emergencia
			conveyorForward(); //Encendemos la cinta con su señal PWM.

			if(presencia == 1){ //Si se detecta una botella (boton azul)
				conveyorStop(); //Paramos la cinta
				contador = 0; //El contador del timer 2 vuelve a 0.
	            __HAL_TIM_SET_COUNTER(&htim2, 0); //Comenzamos el temporizador 2 desde 0 por si acaso se ha desincronizado.
	            HAL_TIM_Base_Start_IT(&htim2); //Y lo encendemos en esta linea.

	            valveOpen(); // Damos la orden oficial para que la valvula haga su faena.

	            estado = 1; //Cambiamos al estado 1
	            presencia = 0; //En la industria esto no se pondría, simplemente habria que condicionar el bloque a un flanco de subida
	            //Con un if superior de presencia==1 y un if inferior de presencia!=presencia_anterior
	            //Sin embargo como es un boton da bastante igual, nos ahorramos poner un if inferior de presencia_anterior.
				}
				break;

		case 1: // ABRIENDO VÁLVULA

			if(contador >= 100){ //Cuando el contador del timer cuenta 100 o mas..... 10 ms * 100 = 1000 ms
				contador = 0;//Reseteamos el contador para el siguiente estado
				estado = 2;//Cambiamos de estado
			}
			break;

		//El case 2 y 3 hacen lo mismo que el 1.

		case 2: // VÁLVULA ABIERTA
			if(contador >= 100){
				contador = 0;
				estado = 3;
			}
			break;

		case 3: // CERRANDO VÁLVULA
			if(contador >= 100){
				contador = 0;
				estado = 0; // Terminado, volvemos a mover la cinta
			}
			break;

		case 4: // !!! EMERGENCIA !!! La lógica de activación del case 4 se encuentra en el archivo \USB_DEVICE\App\usbd_cdc_if.c CDC_Recieve_FS();
			conveyorStop(); //Paramos la cinta

	        valveClose(); //Cerramos la válvula de golpe.

			HAL_TIM_Base_Stop_IT(&htim2); //Apagamos temporizador para evitar que se desincronice.

			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //Encendemos Led roja de emergencia.
			break;

	}
}
