# Laboratorio_C_grupoA15
# Artemis Simulator (Órbita Baja) — Situación-Problema 2

**Trabajo Final Integrador — Laboratorio de C**
Universidad Tecnológica Nacional, Facultad Regional Resistencia
Cátedra: Algoritmos y Estructuras de Datos | Ingeniería en Sistemas de Información

---

## 1. Integrantes y comisión

| Nº | Apellido y nombre | Legajo | Usuario de GitHub | Parte principal |
|----|-------------------|--------|-------------------|-----------------|
| 1 | Alvira Coseres, Valentina| 31642 |   |   |
| 2 | Alfonso Abatte, Valentina| 31059 |  |   |
| 3 | Segovia,Sebastian| [COMPLETAR] | [COMPLETAR] | [COMPLETAR] |
| 4 |  Saade,Toni | [COMPLETAR] | [COMPLETAR] | [COMPLETAR] |

- Grupo: A15
- Comisión:  k1.1
- Repositorio: https://github.com/valen-tina-Alvira/Laboratorio_C_grupoA15

---

 2. Escenario asignado y reglas del juego

# Situación-Problema 2: Inserción y estabilidad en órbita terrestre baja (LEO)

Tras apagar sus motores de ascenso, la nave **Orion** queda en una órbita circular de estacionamiento a **h = 192 km** sobre la superficie terrestre, describiendo un **Movimiento Circular Uniforme (MCU)**. El simulador comienza en ese instante.

**Datos base del problema**

| Dato                                 | Valor            |
|------                                |-------           |
| Masa de la Tierra (Mt)               | 5,972 × 10²⁴ kg  |
| Radio medio de la Tierra (Rmt)       | 6.371.000 m      |
| Altura orbital (h)                   | 192.000 m |
| Radio orbital (Ro = Rmt + h)         | 6.563.000 m |
| Constante de gravitación (G)         | 6,674 × 10⁻¹¹ N·m²/kg² |
| Masa de la nave (Mn)                 | 33.000 kg |

 Física representada

| Concepto                                              | Fórmula |
|----------|                                             ---------|
| Atracción gravitatoria (actúa como fuerza centrípeta) | Fg = G · Mt · Mn / Ro² |
| Velocidad orbital ideal (Fg = Fc)                     | v = √(G · Mt / Ro) |
| Período orbital                                       | T = 2π · Ro / v |
| Aceleración centrípeta                                | ac = v² / Ro |
| Velocidad de escape                                   | v_esc = √2 · v_ideal |
| Velocidad angular                                     | Ω = v / Ro |
| Velocidad lateral (propulsor auxiliar)                | v_lat = a · Δt |
| Desviación lateral                                    | Δy = ½ · a · Δt² |
  Estados posibles de la órbita (con margen de tolerancia de ±50 m/s)

| Condición sobre la velocidad v | Resultado |
|--------------------------------|-----------|
Caída: la nave reingresa a la Tierra | v < v_ideal − 50 m/s |
Estable: órbita circular | \|v − v_ideal\| ≤ 50 m/s 
Elíptica: la órbita se deforma |v_ideal + 50 m/s < v < v_escape
Escape: la nave abandona la gravedad terrestre |v ≥ v_escape

  Parámetros que puede modificar el usuario (todos validados)

| Parámetro | Rango | Valor predefinido |
|-----------|-------|-------------------|
| Masa de la nave | [10.000 kg ; 50.000 kg] | 33.000 kg |
| Altura orbital | [160.000 m ; 2.000.000 m] | 192.000 m |
| Cantidad de vueltas | [1 ; 10] | 1 |
| Aceleración lateral | [0 m/s² ; 15 m/s²] | 0,50 m/s² (demo) |
| Duración del pulso lateral | [0 s ; 60 s] | 10 s (demo) |
| Velocidad inicial (prueba de estabilidad) | [6.500 m/s ; 11.200 m/s] | — |

Reglas de coherencia: si la aceleración lateral es 0, la duración se fija automáticamente en 0. Si es mayor que 0, la duración debe ser mayor que 0, porque no existe una aceleración instantánea.

---

  3. Funcionamiento

### Menú principal

1. Breve descripción del programa Artemis.
2. Descripción del simulador (objetivo, variables, física).
3. **Iniciar simulador** (núcleo del programa).
4. Consultar historial de simulaciones.
5. Salir.

### Submenú de la opción 3

| Modo | Qué hace |
|------|----------|
| Simulación estándar | Usa los datos predefinidos del Problema 2 y anima la órbita. |
| Simulación personalizada | El usuario ingresa masa, altura, vueltas, aceleración y tiempo; cada dato se valida antes de continuar. |
| Prueba de estabilidad | El usuario ingresa la velocidad inicial y se clasifica el resultado (estable, caída, elíptica o escape). |
| Demostraciones de conceptos físicos | Las preguntas del TP: se ejecuta primero la simulación y luego se muestra el desarrollo con los cálculos. |
| Volver al menú principal | — |

### Animación en consola

- Matriz de **61 × 25** caracteres, con la Tierra vista desde un polo en el centro **(30, 12)**.
- La nave se dibuja como `#` y deja un rastro de puntos `.` con la trayectoria.
- Se corrige la proporción de la consola multiplicando la coordenada x por 2.
- El tiempo real de la órbita se acelera para que la animación dure **como máximo 60 segundos**.

### Historial

Se guardan hasta **20 simulaciones**. Al llegar al máximo, la siguiente elimina el registro de la posición 0 y desplaza los demás (`i = i − 1`), de modo que la nueva queda en la posición 19. Cada registro conserva el modo, los datos de entrada y el resultado.

### Arquitectura del proyecto

```
Laboratorio_C_grupoA15/
├── main.c            # main(), menú principal y submenú
├── artemis.h         # constantes, enum, structs y prototipos
├── artemis.c         # cálculos físicos, estabilidad, animación, historial, demos
├── validaciones.h    # prototipos de entrada segura e interfaz
├── validaciones.c    # lectura validada de enteros/reales, limpiar pantalla, pausas
├── Makefile
└── README.md
```

```
            +-----------+
            |  main.c   |  menús y flujo general
            +-----+-----+
                  |  usa
        +---------+----------+
        v                    v
 +-------------+      +----------------+
 |  artemis.c  |----->| validaciones.c |
 |  (lógica)   | usa  | (entrada/UI)   |
 +------+------+      +----------------+
        |
        v
  artemis.h  (structs: SistemaOrbital, RegistroSimulacion,
              HistorialSimulaciones; enum EstadoOrbital)
```

**Estructuras de datos principales**

- `SistemaOrbital`: agrupa entradas, magnitudes calculadas, posición en la animación y resultado. Se pasa por **puntero** entre funciones para no duplicar datos.
- `RegistroSimulacion`: una ficha del historial.
- `HistorialSimulaciones`: arreglo de 20 registros más sus contadores.
- `EstadoOrbital`: `enum` con los cuatro resultados posibles.

### Validaciones

Todas las lecturas limpian el buffer de entrada, así que ingresar letras u otros caracteres no provoca bucles infinitos ni cierres inesperados. Los datos fuera de rango se rechazan y se vuelve a pedir el dato con un mensaje claro.

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
