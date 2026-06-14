#include "main.h"
#include "bottler.h"

extern ADC_HandleTypeDef hadc1; // Importamos del codigo fuente generado por la libreria HAL nuestro ADC.
extern TIM_HandleTypeDef htim4; // Importamos del codigo fuente generado por la libreria HAL nuestro timer.
volatile uint8_t forward = 0;

void bottlerInit(void){

	// Arrancamos el PWM de la válvula (Naranja - Canal 2)
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

	// ¡AHORA SÍ! Arrancamos el PWM de la cinta (VERDE - Canal 1)
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

	/* Toda la señal PWM la genera el hardware del TIM4 por sus canales (configurado en el .ioc a 1kHz).
	Su modus operandi es variar el ciclo de trabajo (Duty Cycle). A mayor ciclo de trabajo,
	veremos el LED brillar con más intensidad, ya que el ojo humano no percibe parpadeos a 1000 Hz,
	sino que integra la luz media. */
}

void conveyorForward(){
    forward = 1;

    // USAMOS EL LED VERDE PARA LA CINTA CON SEÑAL PWM (Canal 1)
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 800);

    /* &htim4 es el registro del timer y el canal 1 es la salida física.
    El 800 NO son milisegundos, es el valor de comparación contra el registro ARR (que vale 1000).
    Como 800/1000 = 0.8, logramos un Duty Cycle del 80%. El pulso está en alto el 80% de un milisegundo
    (0.8 ms) y en bajo el 20% restante. Esto se traduce en un 80% de brillo real en el LED. */
}

void conveyorStop(){
    forward = 0;

    // APAGAMOS EL LED VERDE (Canal 1 a 0%)
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    // Lo mismo que antes, pero un ciclo de trabajo del 0% equivale a forzar la señal a 0V continuamente.
}

void valveOpen(void){
    // La rampa lógica está programada en el TIM2_IRQHandler (stm32f4xx_it.c).
    // Aquí simplemente nos aseguramos de que el PWM parta de cero exacto al dar la orden.
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
}

void valveClose(void){
    // Cierre de emergencia o fin de ciclo. Corta el PWM del LED naranja al instante (Duty Cycle = 0%).
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
}

float getTemperature(void) {
    // Valor por defecto para evitar basura en memoria.
    // No lleva static para que se cree en la pila y se destruya al salir de la función.
    float temperatura = 25.0f;

    HAL_ADC_Start(&hadc1); // Disparamos la lectura por hardware.

    // Esperamos máximo 10ms a que termine de leer (que el hardware devuelva HAL_OK).
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {

        // Recogemos la muestra. Al ser de 12 bits (0 a 4095), necesitamos al menos 16 bits,
        // y la HAL nos lo devuelve en formato uint32_t.
        uint32_t adc_val = HAL_ADC_GetValue(&hadc1);

        // 1. Convertimos el valor digital (0-4095) a Voltios reales (asumiendo alimentación de 3.3V)
        float vsense = (adc_val * 3.3f) / 4095.0f;

        // 2. Aplicamos la fórmula del Datasheet del STM32F4 para el sensor de temperatura interno:
        // V25 = 0.76V (voltaje a 25ºC), Avg_Slope = 0.0025 V/ºC (2.5 mV por grado)
        temperatura = ((vsense - 0.76f) / 0.0025f) + 25.0f;

        // Esto es una conversión Analógica-Digital (ADC) clásica de manual para leer un sensor físico.
    }

    // Paramos el periférico para ahorrar energía y no saturar el bus leyendo si no se lo pedimos.
    HAL_ADC_Stop(&hadc1);

    return temperatura;
}
