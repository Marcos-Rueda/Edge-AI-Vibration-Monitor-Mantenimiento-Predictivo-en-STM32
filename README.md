# Edge AI Vibration Monitor: Mantenimiento Predictivo en STM32

Sistema completo de mantenimiento predictivo en el borde (*Edge AI*) desarrollado sobre el microcontrolador **STM32L475VGTx** (ARM Cortex-M4). Este proyecto detecta en tiempo real anomalías mecánicas en maquinaria rotativa (como un ventilador) utilizando Machine Learning embebido.

El repositorio abarca el ciclo completo de ingeniería: desde el firmware de adquisición inercial y generación de datasets por puerto serie, hasta el despliegue de la librería de Machine Learning en C y el control en tiempo real de actuadores visuales (LEDs RGB) mediante PWM y DMA.

## 🗂️ Estructura del Repositorio

El proyecto está dividido en dos carpetas principales que corresponden a las dos etapas del flujo de trabajo:

* **`NanoEdge_DataLog/` (Fase 1 - Adquisición de Datos):** 
  Contiene el firmware encargado de muestrear el acelerómetro y volcar los datos por UART en formato CSV para crear los datasets de entrenamiento (`regular` y `anomaly`).
* **`NanoEdge_Detection/` (Fase 2 - Inferencia On-Device):** 
  Contiene el firmware final. Incluye la librería estática generada por NanoEdge AI Studio (`libneai.a`), la lógica de detección de anomalías y el control visual del anillo LED.

## 🚀 Características Técnicas Destacadas

* **Edge AI Ultra-Compacto:** Integración de la librería de Machine Learning con un **100% de Balanced Accuracy**. Consumo de memoria extremadamente bajo: ~0.8 KB de RAM y ~0.4 KB de Flash.
* **Adquisición Inercial Precisa:** Muestreo de los ejes X, Y y Z del acelerómetro MEMS **LSM6DSL** a **416 Hz** en bloques de 64 muestras por eje para analizar el espectro vibratorio.
* **Driver WS2812 optimizado (0% CPU Overhead):** Control del anillo de 6 LEDs RGB mediante **PWM + DMA**. Generación de trama serie a 800 kHz utilizando el Timer 2 y el DMA1, evitando cuelgues o retardos en el procesador.
* **DataLogger Serie:** Redirección de `printf` hacia `USART1` (115200 bps) con soporte para coma flotante (`newlib-nano`) para la recolección limpia de datos.

## 🛠️ Arquitectura Hardware

* **Microcontrolador:** STM32L475VGTx (ARM Cortex-M4 con FPU) operando a 80 MHz.
* **Sensor (LSM6DSL):** Acelerómetro 3D conectado vía `I2C2` (Pines `PB10` y `PB11`).
* **Actuador (WS2812):** Anillo LED conectado a `TIM2_CH1` gestionado por `DMA1_CH5`.
* **Comunicación:** UART asíncrona a 115200 bps para telemetría y debug.

## ⚙️ Funcionamiento del Firmware (Detección)

1. **Fase de Aprendizaje In Situ:** Al arrancar, el sistema realiza iteraciones de aprendizaje llamando a la API de NanoEdge para memorizar el patrón de vibración normal del ventilador. Durante esta fase, el anillo LED muestra una animación azul.
2. **Detección Continua:** En el bucle principal, se capturan nuevas ventanas de datos y se evalúan. 
   * **Estado Nominal:** Si la similitud con el patrón normal es alta (>= 90%), el sistema notifica "Regular" y mantiene una iluminación nominal.
   * **Detección de Anomalía:** Si la similitud cae por debajo del umbral debido a una obstrucción o fallo mecánico, el sistema alerta "Abnormal" por el puerto serie y conmuta instantáneamente todos los LEDs a color rojo brillante.

## 💻 Instrucciones de Despliegue

1. Clona este repositorio en tu equipo local.
2. Abre **STM32CubeIDE** e importa los proyectos desde las carpetas `NanoEdge_DataLog` o `NanoEdge_Detection`.
3. Para el proyecto de detección, asegúrate de que el enlazador (Linker) está configurado para incluir la librería estática `libneai.a` y tiene habilitado el soporte de coma flotante para `printf`.
4. Compila y flashea el firmware en la placa STM32.
5. Inicia la máquina monitorizada (ventilador) inmediatamente después de encender la placa para que el aprendizaje inicial se realice sobre el estado nominal correcto.