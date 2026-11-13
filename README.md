# Laboratorio_C_grupoA15
# Artemis Simulator (Órbita Baja) — Situación-Problema 2

**Trabajo Final Integrador — Laboratorio de C**
Universidad Tecnológica Nacional, Facultad Regional Resistencia
Cátedra: Algoritmos y Estructuras de Datos | Ingeniería en Sistemas de Información

Simulador interactivo en lenguaje C de la fase de inserción de la nave Orion en una órbita terrestre baja (LEO), dentro de la misión Artemis. Los datos se ingresan por consola y la simulación se muestra en una ventana gráfica (raylib).
---

## 1. Integrantes y comisión

| Nº | Apellido y nombre | Legajo | Usuario de GitHub |
|----|-------------------|--------|-------------------|
| 1 | Alvira Coseres, Valentina| 31642 |   |
| 2 | Alfonso Abatte, Valentina| 31059 |  |
| 3 | Segovia,Sebastian| [COMPLETAR] | [COMPLETAR] |
| 4 |  Saade,Antonio | [COMPLETAR] | [COMPLETAR] |

- Grupo: A15
- Comisión:  k1.1
- Repositorio: https://github.com/valen-tina-Alvira/Laboratorio_C_grupoA15

---

 2. Escenario asignado y reglas del juego

# Situación-Problema 2: Inserción y estabilidad en órbita terrestre baja (LEO)

Tras apagar sus motores de ascenso, la nave **Orion** queda en una órbita circular de estacionamiento a **h = 192 km** sobre la superficie terrestre, describiendo un **Movimiento Circular Uniforme (MCU)**. El simulador comienza en ese instante.

**2.1 Datos base del problema**

| Dato                                 | Valor            |
|------                                |-------           |
| Masa de la Tierra (Mt)               | 5,972 × 10²⁴ kg  |
| Radio medio de la Tierra (Rmt)       | 6.371.000 m      |
| Altura orbital (h)                   | 192.000 m |
| Radio orbital (Ro = Rmt + h)         | 6.563.000 m |
| Constante de gravitación (G)         | 6,674 × 10⁻¹¹ N·m²/kg² |
| Masa de la nave (Mn)                 | 33.000 kg |

El programa responde, para los datos que ingrese el usuario, las cuatro preguntas de análisis del enunciado:
 
1. Demostrar que la fuerza de atracción gravitatoria actúa como fuerza centrípeta y calcular la velocidad orbital.
2. Calcular el período orbital T en segundos y minutos.
3. Calcular la aceleración centrípeta.
4. Ante un pulso de aceleración lateral (a = 0,50 m/s² durante Δt = 10 s, perpendicular a la velocidad), describir la trayectoria y calcular la desviación lateral.
 
### 2.2 Variables que puede modificar el usuario
 
Todas las entradas se validan; si el dato es inválido o está fuera de rango, el programa lo informa y lo vuelve a pedir.
 
| Variable | Rango permitido |
|---|---|
| Masa de la nave Mn | [10.000 ; 50.000] kg |
| Altura orbital h | [160.000 ; 2.000.000] m |
| Cantidad de vueltas | [1 ; 10] (entero) |
| Aceleración lateral a | [0 ; 15] m/s² |
| Duración del pulso t | [0 ; 60] s |
| Velocidad inicial (prueba de estabilidad) | [6.500 ; 11.200] m/s |

Reglas adicionales de ingreso:
 
- Si **a = 0**, la duración del pulso se asigna automáticamente a 0 s.
- Si **a > 0**, la duración **t debe ser mayor que 0**.
- Los números reales se escriben con **punto** decimal (por ejemplo `3.5`); la coma se rechaza con un mensaje.
- No se aceptan letras, notación científica (`1e5`) ni textos mezclados con números (`12abc`).

### 2.3 Clasificación de la órbita
 
El programa compara la velocidad utilizada **v** con la velocidad orbital ideal **vi = √(G·MT / r)** y con la velocidad de escape **vesc = √2 · vi**, con una tolerancia de ±50 m/s:
 
| Resultado | Condición |
|---|---|
| **Caída** | v < vi − 50 m/s |
| **Estable** (órbita circular) | vi − 50 ≤ v ≤ vi + 50 m/s |
| **Elíptica** | vi + 50 m/s < v < vesc |
| **Escape** | v ≥ vesc |

### 2.4 Historial
 
El programa guarda hasta **20 simulaciones** en un arreglo de registros. Al ejecutar la simulación número 21, se elimina el registro más antiguo (posición 0) y todos los demás se corren una posición (`i = i − 1`), quedando la nueva simulación en la última posición. Cada registro guarda: ID correlativo, modo, masa, altura, velocidad, período, aceleración y duración del pulso, desviación lateral, vueltas y resultado.
 
---
## 3. Funcionamiento
 
### 3.1 Flujo del programa
 
1. **Menú principal:** descripción de Artemis, descripción del simulador, iniciar simulador, consultar historial y salir.
2. **Submenú del simulador (opción 3):**
   - **Simulación estándar:** usa los datos predefinidos del Problema 2.
   - **Simulación personalizada:** el usuario ingresa masa, altura, vueltas y pulso lateral.
   - **Prueba de estabilidad:** el usuario ingresa solo la velocidad inicial y se evalúa si la órbita resulta estable, de caída, elíptica o de escape.
   - **Demostraciones de conceptos físicos:** muestra la simulación y luego las cuatro respuestas analíticas con *fórmula → reemplazo → resultado*.
3. En cada modo se calcula la física, se abre la **ventana gráfica** con la animación (Tierra, órbita prevista, rastro y nave, con un panel de datos en vivo), se guarda la corrida en el historial y se muestra un **reporte final** en consola.
### 3.2 Fórmulas utilizadas
 
| Magnitud | Fórmula |
|---|---|
| Radio orbital | Ro = RT + h |
| Fuerza gravitatoria | Fg = G · MT · Mn / Ro² |
| Fuerza centrípeta | Fc = Mn · v² / Ro |
| Velocidad orbital (de igualar Fg = Fc) | v = √(G · MT / Ro) |
| Velocidad de escape | vesc = √2 · v |
| Período orbital | T = 2π · Ro / v |
| Aceleración centrípeta | ac = v² / Ro |
| Velocidad angular | ω = v / Ro |
| Velocidad transversal del pulso | vlat = a · t |
| Desviación lateral | dy = ½ · a · t² |
 
### 3.3 Arquitectura y modularización
 
| Módulo | Responsabilidad |
|---|---|
| `main.c` | Función `main()`, menú principal, submenú del simulador y flujo general |
| `artemis.h` | Constantes (`#define`), tipos (`enum`, `struct`) y prototipos de las funciones públicas |
| `artemis.c` | Lógica de negocio: cálculos físicos, clasificación de la órbita, configuración de modos, historial, reportes y demostraciones |
| `simulador.c` | Actualización del estado cuadro a cuadro y animación gráfica (versión con ventana raylib y versión de respaldo en consola) |
| `validaciones.c/.h` | Ingreso seguro de enteros, reales y respuestas S/N; regla combinada del pulso lateral; pausas |
| `Makefile` | Compilación del proyecto |
| `raylib/` | Librería gráfica raylib (cabeceras y librería estática) |
 
Estructuras de datos principales:
 
- `SistemaOrbital`: estado completo de una simulación (entradas, magnitudes calculadas, posición y resultado).
- `RegistroSimulacion`: una ficha del historial.
- `HistorialSimulaciones`: arreglo de 20 registros con sus contadores.
- `EstadoOrbital` (`enum`): Estable, Caída, Elíptica, Escape.
Las funciones reciben la estructura **por puntero** para modificar el estado original sin copiarlo, y usan `const` cuando solo la leen.
 
### 3.4 Decisiones de diseño a tener en cuenta
 
- **Escala visual:** en la ventana, la órbita se dibuja con un radio de entre 7 y 10 "filas" y la Tierra con 5, para que la altura sea visible. No está a escala real (a 192 km, la órbita real está apenas un 3 % por encima de la superficie).
- **Tiempo acelerado:** un período real dura unos 88 minutos; la animación acelera el tiempo para no superar 60 segundos reales.
- **Tolerancia de ±50 m/s:** es una simplificación del modelo para clasificar la órbita como estable.
- **Pulso lateral:** se informan la velocidad transversal, la desviación lateral, el módulo de la velocidad resultante y el giro de su dirección. Con los datos por defecto el cambio de la velocidad es muy pequeño, por lo que la órbita se mantiene prácticamente circular.
---
  

## 4. Compilación y ejecución

  

---

## 5. Registro del uso de Inteligencia Artificial

Herramientas utilizadas: **Claude (Anthropic)** — [COMPLETAR si usaron otras: ChatGPT, Copilot, Gemini, etc.]

| Fecha | Herramienta | Tarea | Prompt / consulta | Cómo se validó y adaptó |


**Criterio del equipo:** todo código sugerido por una IA fue leído, probado y entendido por los integrantes antes de incorporarlo, y cada integrante puede explicar cualquier parte del programa. [COMPLETAR o ajustar según lo que realmente hicieron]

---

## 6. Pruebas realizadas

| Entrada | Resultado esperado |
|---------|--------------------|
| Letras donde se pide un número | Mensaje de error y se vuelve a pedir el dato |
| Opción de menú fuera de rango | Mensaje de error y se vuelve a pedir |
| Masa de 5.000 kg o 60.000 kg | Rechazada (rango 10.000–50.000 kg) |
| Aceleración > 0 con tiempo = 0 | Rechazada (el tiempo debe ser mayor que 0) |
| Velocidad 6.500 m/s en prueba de estabilidad | Caída |
| Velocidad 7.792,93 m/s | Órbita estable |
| Velocidad 8.500 m/s | Órbita elíptica |
| Velocidad 11.200 m/s | Escape |
| 21 simulaciones seguidas | El historial conserva solo las últimas 20 |

[COMPLETAR: agregar los resultados reales obtenidos]
