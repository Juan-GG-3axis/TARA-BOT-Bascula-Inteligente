# TARA-BOT: Báscula Inteligente para Filamento 3D

Este repositorio contiene los archivos necesarios para construir el **TARA-BOT**, un sistema de pesaje inteligente para impresoras 3D basado en un ESP32. El dispositivo resta automáticamente el peso del carrete vacío (tara) y muestra el filamento restante a través de una interfaz gráfica que cambia de expresión facial (feliz, serio, enojado, triste, etc.) según la cantidad de material disponible.

## Archivos Incluidos
*   **Código de Calibración:** Script ligero para obtener el factor de calibración exacto de tu celda de carga usando la pantalla TFT.
*   **Código Principal:** El firmware completo con la máquina de estados, animaciones geométricas (Sprites), gestión de memoria flash y lectura asíncrona del sensor.
*   **User_Setup.h:** Archivo de configuración modificado indispensable para la librería `TFT_eSPI`, optimizado para el controlador GC9A01 y los pines del ESP32.

## Hardware y Conexiones (Pinout)
*   **Placa:** ESP32 (DevKit)
*   **Pantalla:** TFT Circular 1.28" (GC9A01)
    *   CS: Pin 15
    *   DC: Pin 2
    *   RST: Pin 4
    *   SDA/MOSI: Pin 23
    *   SCL/SCK: Pin 18
*   **Celda de Carga:** Módulo amplificador HX711
    *   DOUT: Pin 21
    *   SCK: Pin 22
*   **Control:** Push Button
    *   Pin 33 (INPUT_PULLUP)

## Dependencias (Librerías Arduino)
*   `TFT_eSPI` (por Bodmer) - Para el manejo gráfico en memoria RAM sin parpadeos.
*   `HX711 Arduino Library` (por Bogdan Necula) - Para la lectura del sensor de peso.

## Guía Rápida de Instalación
1.  **Configura la Pantalla:** Tras instalar la librería `TFT_eSPI`, ve a la carpeta de instalación (usualmente `Documentos/Arduino/libraries/TFT_eSPI/`), elimina el archivo `User_Setup.h` original y reemplázalo por el que se incluye en este repositorio.
2.  **Calibra tu Sensor:** Sube el Código de Calibración a tu ESP32. Sigue las instrucciones en pantalla y utiliza un objeto de peso conocido para calcular tu factor de calibración mediante la fórmula: `Factor = Valor Crudo / Peso Real (g)`.
3.  **Sube el Código Principal:** Abre el Código Principal, busca la instrucción `scale.set_scale(430.1);` y reemplaza el número por tu nuevo factor de calibración. Si lo deseas, modifica la matriz de taras con los pesos reales de tus carretes vacíos. Sube el código al ESP32.

Para ver el tutorial completo paso a paso, diagrama de ensamblaje en 3D y explicación detallada del código, visita el artículo completo en el blog de **Triaxis Point**.
