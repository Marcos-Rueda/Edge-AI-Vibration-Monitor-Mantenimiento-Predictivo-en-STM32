# Edge AI Vibration Monitor: Mantenimiento Predictivo en STM32

Sistema embebido de mantenimiento predictivo en tiempo real que ejecuta algoritmos de Machine Learning directamente en el nodo sensor (*Edge AI*) para detectar anomalías mecánicas en maquinaria rotativa (ventilador) sin depender de procesamiento en la nube. El proyecto abarca desde la captura de datos inerciales y generación del dataset hasta el despliegue de la librería de inferencia en C y la señalización visual mediante periféricos controlados por acceso directo a memoria (DMA).

---

## Estructura del Repositorio

El proyecto está dividido en dos workspaces de **STM32CubeIDE** que corresponden a las dos etapas del flujo de trabajo:

```text
├── NanoEdge_DataLog/                  # Proyecto 1: Captura de datos y generación de CSV
│   └── Core/
│       ├── Inc/
│       │   ├── LSM6DSL.h              # Cabecera del driver I2C del acelerómetro
│       │   └── main.h
│       └── Src/
│           ├── LSM6DSL.c              # Implementación de lectura inercial 3D
│           └── main.c                 # Bucle de muestreo por ventanas y volcado UART
│
└── NanoEdge_Detection/                # Proyecto 2: Inferencia On-Device y actuación DMA
    └── Core/
        ├── Inc/
        │   ├── LSM6DSL.h              # Cabecera del driver del acelerómetro
        │   ├── NanoEdgeAI.h           # API de aprendizaje y detección de anomalías
        │   ├── WS2812.h               # Cabecera del driver para anillo LED RGB
        │   └── main.h
        └── Src/
            ├── libneai.a              # Librería estática precompilada (Cortex-M4)
            ├── LSM6DSL.c              # Driver I2C del sensor LSM6DSL
            ├── WS2812.c               # Controlador PWM + DMA a 800 kHz para LEDs WS2812
            └── main.c                 # Máquina de estados (Learning + Inference)
```

---

## Características Principales

* **Inferencia y Aprendizaje On-Device:** Integración de la librería estática precompilada `libneai.a` generada con NanoEdge AI Studio para ejecutar tanto el entrenamiento dinámico *in situ* como la detección continua de anomalías en un núcleo ARM Cortex-M4.
* **Adquisición de Firmas Vibratorias:** Muestreo de aceleración tridimensional (ejes X, Y, Z) mediante ventanas temporales de 64 muestras consecutivas a 416 Hz con el sensor MEMS LSM6DSL.
* **Control de Actuadores con 0% de Carga de CPU:** Driver para anillo de 6 LEDs RGB direccionables WS2812 operando a una frecuencia estricta de 800 kHz (150 ns) mediante la combinación de **TIM2 (PWM) + DMA**, eliminando el *jitter* por interrupciones del sistema.
* **DataLogger Serie Integrado:** Extracción de ventanas inerciales formateadas en CSV a través de UART a 115200 bps, sobrescribiendo `__io_putchar` con soporte para punto flotante (`newlib-nano`).

---

## Arquitectura Hardware y Configuración de Periféricos

El firmware opera con el reloj del sistema (`SYSCLK`) configurado a su frecuencia máxima de **80 MHz** a través del PLL interno.

| Periférico / Componente | Interfaz / Pines | Configuración Técnica | Función en el Sistema |
| :--- | :--- | :--- | :--- |
| **Sensor Inercial LSM6DSL** | `I2C2` (`PB10` SCL, `PB11` SDA) | Modo Estándar (100 kHz), ODR a 416 Hz | Captura de vibraciones mecánicas en los ejes X, Y, Z. |
| **Anillo de 6 LEDs WS2812** | `TIM2_CH1` + `DMA1_CH5` | PWM a 800 kHz (`ARR = 100`), DMA *Memory-to-Peripheral* | Interfaz visual de estado (aprendizaje, nominal y alarma). |
| **Terminal PC (ST-Link VCP)** | `USART1` (`PB6` TX, `PB7` RX) | Asíncrono, 115200 bps, 8N1 | Volcado de datasets CSV y telemetría de similitud en tiempo real. |

---

## Flujo de Machine Learning y Rendimiento del Modelo

Para entrenar y validar el clasificador en NanoEdge AI Studio, se capturaron empíricamente dos conjuntos de datos balanceados de 200 ventanas cada uno: `regular_signals.csv` (ventilador operando a velocidad nominal estable) y `anomaly_signals.csv` (desequilibrios en el flujo de aire y fricción física inducida).

El modelo óptimo obtenido tras la fase de *Benchmark* (basado en algoritmos compactos de distancia y análisis espectral) arroja las siguientes métricas de eficiencia en el microcontrolador:

* **Balanced Accuracy:** 100% (0% de falsos positivos en validación).
* **Índice de Calidad:** 100 puntos.
* **Huella de Memoria RAM:** aprox 0.8 KB.
* **Huella de Memoria Flash:** aprox 0.4 KB.

---

## Máquina de Estados del Firmware

El ciclo de vida de la aplicación principal se divide en dos etapas secuenciales haciendo uso de la API de `NanoEdgeAI.h`:

1. **Fase de Aprendizaje Dinámico (*Learning*):** Al arrancar, el sistema recolecta ventanas vibratorias durante las primeras $ N = 20 $ iteraciones (`LEARNING_ITERATIONS`) e invoca `neai_anomalydetection_learn(input_user_buffer)` para parametrizar la línea base del ventilador en su entorno actual. Durante este proceso, el anillo WS2812 muestra una animación rotatoria en color azul (`WS2812_Set_LED(iteration % 6, 0, 0, 255)`).
2. **Fase de Inferencia en Tiempo Real (*Detection*):** En el bucle infinito `while(1)`, el microcontrolador captura nuevas ventanas de forma ininterrumpida y ejecuta `neai_anomalydetection_detect(input_user_buffer, &similarity)`:
   * **Estado Nominal (`similarity >= 90`):** El sistema reporta operación regular por consola serie y mantiene el patrón multicolor/verde en el anillo LED.
   * **Detección de Anomalía (`similarity < 90`):** Ante una desviación vibratoria, el sistema dispara una alerta por puerto serie y conmuta inmediatamente todos los LEDs a rojo vivo (`WS2812_Set_All_LED(255, 0, 0)`).

---

## Cómo Compilar y Ejecutar

1. Clonar el repositorio e importar los proyectos en **STM32CubeIDE**.
2. Verificar en las propiedades del proyecto (`C/C++ Build -> Settings -> GCC Linker -> Libraries`) que la librería estática `libneai.a` está correctamente enlazada y que el flag `Use float with printf from newlib-nano` está activo.
3. Compilar y flashear el binario en la placa STM32 manteniendo el ventilador en su estado de funcionamiento normal para que las 20 iteraciones iniciales aprendan la referencia correcta.
4. Abrir un monitor serie a **115200 bps** para observar el progreso de aprendizaje y el porcentaje de similitud en tiempo real.
