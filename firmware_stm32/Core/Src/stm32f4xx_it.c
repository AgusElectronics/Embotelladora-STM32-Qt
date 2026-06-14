/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim2;
/* USER CODE BEGIN EV */
extern TIM_HandleTypeDef htim4;
extern volatile uint8_t estado;
extern volatile uint8_t presencia;
extern volatile uint8_t contador;
extern volatile uint8_t turno_usb;
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */


//IMPORTANTE LEER ESTE COMENTARIO

/*
 * JUSTIFICACIÓN DEL DISEÑO DE INTERRUPCIONES (NVIC) Y PRIORIDADES
 * He diseñado el sistema con un enfoque de Tiempo Real Estricto, asignando las prioridades de la siguiente manera (menor número = mayor prioridad):

 * 1. TIM2 (Prioridad Máxima): Es nuestro "metrónomo". Se encarga de calcular la rampa matemática de la válvula cada 10ms exactos. Si le diera más
 * prioridad al USB, la recepción de un mensaje retrasaría la cuenta del TIM2 y
 * la válvula daría tirones físicos (Jitter) que es un problema ya estudiado en Informática industrial 1. El control del tiempo manda.

 * * 2. USB / OTG_FS (Prioridad Media): Por aquí nos entra la telemetría y, lo más
 * importante, la Seta de Emergencia ('E'). Necesito que se atienda rapidísimo por seguridad, pero sin llegar a pausar el reloj maestro del TIM2.

 * * 3. EXTI0 / Botón Azul (Prioridad Baja): Es una acción puramente humana. Si el procesador tarda un par de milisegundos más en leer que he pulsado el botón
 * porque estaba ocupado con el TIM2 o el USB, es físicamente imperceptible para nosotros y el proceso sigue fluyendo perfectamente.

 * ¿Y por qué el TIM4 NO tiene interrupción?
 * Básicamente porque lo he delegado al hardware (modo Output Compare para el PWM).Si activara una interrupción para generar las señales a 1 kHz, obligaría a la
 * CPU principal a parar miles de veces por segundo solo para encender y apagar los pines de los LEDs. Eso colapsaría la placa entera. Al hacerlo así, yo solo
 * actualizo el registro (CCR) de vez en cuando desde el TIM2, y dejo que el TIM4 se encargue de eso en segundo plano.*/





void EXTI0_IRQHandler(void) // INTERRUPCIÓN DEL BOTÓN AZUL (Presencia de botella)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  // Condición de seguridad: Solo le hacemos caso al botón si estamos en el estado inicial.
  // Si un operario le da al botón mientas la válvula está echando líquido (estado 1, 2 o 3), el procesador lo ignora.
  if (estado == 0) {
      presencia = 1;
  }

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(B1_Pin); // Resetea el flag de la interrupción en el hardware para que no se quede en bucle.
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void) // NUESTRO METRÓNOMO DEL SISTEMA
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  // El hardware (TIM2) está configurado en el CubeMX para saltar aquí exactamente cada 10ms.
  contador++;

  // CONTROL DINÁMICO DEL PWM (VÁLVULA):
  // En lugar de usar HAL_Delay() que bloquearía la placa, recalculamos el Duty Cycle sobre la marcha.

  if(estado == 1){ // ABRIENDO VÁLVULA (Rampa de 0 a 1 seg)
      // Básicamente, esta función se mete en las tripas del registro del TIM4 (Canal 2) y sube la barrera.
      // En la vuelta 1 (10ms) el PWM vale 10. En la vuelta 50 (500ms) vale 500. Al llegar a 100 vale 1000.
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, contador * 10);
  }
  else if(estado == 2){ // ABIERTA AL MÁXIMO (De 1 a 2 seg)
      // Mantenemos la válvula escupiendo líquido al 100% de apertura física.
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1000);
  }
  else if(estado == 3){ // CERRANDO VÁLVULA (Rampa de 2 a 3 seg)
      // Matemáticas a la inversa: empezamos en 1000 y le vamos restando para cerrar suave.
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 1000 - (contador * 10));
  }
  else if(estado == 0){ // CERRADA TOTALMENTE (Esperando botella)
      // Duty Cycle al 0%. Válvula físicamente apagada.
      __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);
  }

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */

  // Cuando llega voltaje por las líneas D+ y D- del USB,
  // salta esta interrupción y le pasa el paquete a la librería de ST para que lo descifre
  // y termine llamando a nuestra función CDC_Receive_FS().
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);

  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
