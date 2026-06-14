# Automatización de Planta Embotelladora - STM32 & Qt HMI

## 📖 Descripción del Proyecto
Este repositorio contiene el código fuente para automatizar el proceso de una planta embotelladora utilizando la placa de desarrollo STM32F4-Discovery. El sistema simula una cinta transportadora que mueve envases y una válvula que los llena de líquido, gestionado todo mediante una máquina de estados robusta y supervisado a través de una interfaz gráfica (HMI) desarrollada en C++ con Qt.

*[Insertar aquí tu vídeo o GIF mostrando la interfaz gráfica funcionando y la placa]*

## 🚀 Características y Funcionalidades

* **Control por Máquina de Estados:** Implementación en C para gestionar el ciclo de espera, llenado, y evacuación de botellas.
* **Gestión de Interrupciones Externas (EXTI):** Uso del pulsador azul de la placa para detectar la presencia de botellas sin recurrir a polling continuo.
* **Control de Tiempos No Bloqueante:** Utilización del temporizador TIM2 para gestionar los tiempos de la máquina de estados sin bloquear la ejecución.
* **Señales PWM por Hardware:** Generación de rampas graduales para la apertura y cierre de la válvula mediante el temporizador TIM4.
* **Monitorización Térmica Constante:** Lectura del sensor de temperatura interno vía ADC con activación de estado de emergencia automático si se superan los 71°C.
* **Interfaz Gráfica de PC (HMI):** Aplicación de escritorio conectada por USB que muestra el estado de la máquina, cuenta las botellas, grafica las señales PWM en tiempo real y permite activar una seta de emergencia.

## 🛠️ Retos de Ingeniería y Soluciones

Durante el desarrollo nos enfrentamos a problemas reales de integración hardware-software que resolvimos de la siguiente manera:

* **Saturación del Bus USB:** Al intentar enviar a la vez la temperatura y el estado por el puerto serie, el buffer del USB se saturaba (error USBD_BUSY). Lo solucionamos enviando la temperatura y usando la función `HAL_GetTick()` para esperar 100 ms antes de enviar el estado, evitando bloqueos.
* **Recuperación Segura del Sistema:** Al salir de una alarma térmica (tras parar el TIM2), la placa podía bloquearse si la alarma saltaba en reposo. Modificamos la máquina de estados para que, al salir de la emergencia, la botella actual se descarte y el sistema regrese siempre directamente al Estado 0 (espera de botella) reactivando el temporizador.
* **Detección Automática de Puertos COM:** Para evitar que la interfaz en Qt intentara conectarse por error al grabador de la placa (ST-Link), desarrollamos un filtro de texto en el buscador de puertos de Qt que ignora la palabra "STLink" y busca el siguiente puerto válido.

## 🏗️ Arquitectura de Software (C++ Qt)
El código de la interfaz gráfica está modularizado en tres clases principales:
* **`SerialManager`:** Se encarga únicamente de abrir el puerto serie y gestionar la comunicación USB con la placa.
* **`PwmOscilloscope`:** Dibuja las gráficas de las rampas del PWM basándose en el estado de la máquina recibido, evitando saturar el puerto serie.
* **`MainWindow`:** Muestra los iconos de los estados, actualiza la barra de temperatura y aumenta el contador de botellas cada vez que la válvula termina de cerrarse.

## 👨‍💻 Equipo de Desarrollo
* Carlos David Saiz Jiménez
* Agustín Muñóz Santucho
* Alejandro Silla González
* [cite_start]Agustín Muñóz Santucho [cite: 164]
* [cite_start]Alejandro Silla González [cite: 165]
