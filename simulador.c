/* =========================================================================
 * simulador.c - Modulo de actualizacion del estado del juego (animacion)
 * =========================================================================
 * Este archivo completa el "Modulo de Lógica" del punto 5.1 junto con
 * artemis.c (que concentra los calculos fisicos y la logica de negocio):
 * aqui se actualiza el estado de la simulacion cuadro a cuadro.
 *
 * Contiene DOS implementaciones de ejecutar_animacion_2d():
 *
 *   - Con USAR_RAYLIB definido: ventana grafica raylib (libreria de
 *     graficos en C). Dibuja la Tierra, la orbita, el rastro y la nave,
 *     con datos de la simulacion en pantalla (HUD).
 *
 *   - Sin USAR_RAYLIB: version de consola con la matriz 61 x 25 de
 *     caracteres (respaldo si la libreria no esta disponible).
 *
 * La fisica por cuadro es identica en ambas: la funcion fisica_cuadro()
 * avanza el modelo orbital segun el estado (estable, eliptica, caida o
 * escape) usando integracion numerica simple (metodo de Euler). */

#include <stdio.h>      /* Para printf (version de consola) */
#include <stdlib.h>     /* Para system (limpiar pantalla en consola) */
#include <string.h>     /* Para memset (inicializar la matriz de consola) */
#include <math.h>       /* Para sqrt, cos, sin, fabs, round */
#include "artemis.h"    /* Constantes, estructuras y prototipos */

/* =========================================================================
 * FISICA COMUN POR CUADRO (la usan las dos versiones de la animacion)
 * ========================================================================= */

/* Avanza el modelo orbital un intervalo dt_sim segun el estado:
 * - Estable:   MCU puro, radio constante (theta avanza con omega).
 * - Eliptica:  conica r = a(1-e^2)/(1+e cos nu) y ley de areas dnu = L/r^2 dt.
 * - Caida:     Euler radial vr -= mu/r^2 dt, r += vr dt, con giro L = Ro*v.
 * - Escape:    Euler radial hacia afuera con la velocidad residual vis-viva.
 * Devuelve 1 (via *termino) cuando la animacion debe terminar: impacto con
 * la superficie o salida del cuadro. */
static void fisica_cuadro(SistemaOrbital *sistema, double dt_sim,
                          double *r, double *vr, double *nu, int *termino)
{
    double mu = G_UNIVERSAL * MASA_TIERRA;          /* Producto G*Mt (m^3/s^2) */
    double eje_mayor;                               /* Semi-eje mayor de la elipse (m) */
    double excentricidad;                           /* Excentricidad de la elipse */
    double momento_angular = sistema->radio_orbital * sistema->velocidad_orbital; /* L = Ro * v */
    double theta = sistema->angulo_theta;           /* Angulo actual acumulado */

    switch (sistema->resultado_codigo) {
        case ESTADO_ESTABLE:                        /* Movimiento circular uniforme */
            theta += sistema->velocidad_angular * dt_sim;   /* theta crece a ritmo de omega */
            *r = sistema->radio_orbital;            /* Radio constante */
            break;

        case ESTADO_ELIPTICA:                       /* Orbita eliptica (r variable) */
            /* Semi-eje mayor por la ecuacion vis-viva: 1/a = 2/Ro - v^2/mu */
            eje_mayor = 1.0 / (2.0 / sistema->radio_orbital
                               - (sistema->velocidad_orbital * sistema->velocidad_orbital) / mu);
            excentricidad = 1.0 - sistema->radio_orbital / eje_mayor;   /* Parte del perigeo */
            *nu += momento_angular / ((*r) * (*r)) * dt_sim;            /* Ley de areas: L / r^2 */
            *r = eje_mayor * (1.0 - excentricidad * excentricidad)
                 / (1.0 + excentricidad * cos(*nu));   /* Ecuacion de la conica */
            theta = *nu;                            /* El angulo dibujado es la anomalia */
            break;

        case ESTADO_CAIDA:                          /* Caida radial hacia la Tierra */
            *vr -= mu / ((*r) * (*r)) * dt_sim;     /* La gravedad acelera la caida (Euler) */
            *r += (*vr) * dt_sim;                   /* Avanza el radio con la velocidad radial */
            theta += momento_angular / ((*r) * (*r)) * dt_sim;  /* Sigue girando: L se conserva */
            if (*r <= RADIO_MEDIO_TIERRA) {         /* Toco la superficie terrestre */
                *r = RADIO_MEDIO_TIERRA;            /* Clampa el radio al borde del planeta */
                *termino = 1;                       /* Termina la animacion: impacto */
            }
            break;

        case ESTADO_ESCAPE:                         /* Alejamiento radial */
        default:
            if (*vr == 0.0) {                       /* Calcula la velocidad residual una sola vez */
                *vr = sqrt(fabs(sistema->velocidad_orbital * sistema->velocidad_orbital
                                - 2.0 * mu / sistema->radio_orbital)); /* v^2 - 2*mu/Ro > 0 */
            }
            *vr -= mu / ((*r) * (*r)) * dt_sim;     /* La gravedad frena el alejamiento (Euler) */
            if (*vr < 0.0) *vr = 0.0;               /* Nunca vuelve a acercarse en este modelo */
            *r += (*vr) * dt_sim;                   /* Avanza el radio hacia afuera */
            theta += momento_angular / ((*r) * (*r)) * dt_sim;  /* Sigue girando mientras se aleja */
            if (*r > 10.0 * sistema->radio_orbital / 7.0) { /* Radio maximo del area visible (10 filas) */
                *termino = 1;                       /* Termina la animacion: fuera de vista */
            }
            break;
    }

    sistema->angulo_theta = theta;                  /* Guarda el angulo actual en la estructura */
    sistema->pos_x_real = (*r) * cos(theta);        /* Coordenada fisica X (m) */
    sistema->pos_y_real = (*r) * sin(theta);        /* Coordenada fisica Y (m) */
}

/* Calcula los tiempos de la animacion: cuadros totales y segundos simulados
 * por cuadro, acelerando el tiempo real para no superar MAX_TIEMPO_ANIMACION. */
static void preparar_tiempo(const SistemaOrbital *sistema,
                            int *frames_total, double *dt_sim)
{
    double duracion_anim;                           /* Duracion real total de la animacion (s) */
    double duracion_simulada;                       /* Tiempo simulado total (s) */

    duracion_simulada = sistema->periodo_segundos * sistema->cantidad_vueltas;
    if (duracion_simulada > MAX_TIEMPO_ANIMACION) { /* Si el periodo real supera el tope */
        duracion_anim = MAX_TIEMPO_ANIMACION;       /* Se acelera el tiempo hasta 60 s reales */
    } else {
        duracion_anim = duracion_simulada;          /* Ya es mas corta que el tope */
    }
    *frames_total = (int)(duracion_anim * 1000.0 / FRAME_MS);   /* Cuadros segun la pausa entre cuadros */
    if (*frames_total < 100) *frames_total = 100;   /* Minimo para que se vea el movimiento */
    if (*frames_total > 900) *frames_total = 900;   /* Maximo razonable de cuadros */
    *dt_sim = duracion_simulada / (double)(*frames_total);      /* Segundos simulados por cuadro */
}

/* Devuelve el color asociado a cada estado de la orbita (solo con raylib) */
#ifdef USAR_RAYLIB
#include "raylib.h"     /* Libreria de graficos: ventanas, dibujado 2D y texto */

static Color color_estado(EstadoOrbital estado)
{
    switch (estado) {                       /* Un color por resultado de la orbita */
        case ESTADO_ESTABLE:  return GREEN;     /* Orbita circular sana */
        case ESTADO_CAIDA:    return RED;       /* Peligro: reingreso */
        case ESTADO_ELIPTICA: return ORANGE;    /* Orbita deformada */
        case ESTADO_ESCAPE:   return PURPLE;    /* Alejamiento definitivo */
        default:              return GRAY;
    }
}

/* ================= VERSION CON VENTANA GRAFICA (RAYLIB) ================= */
void ejecutar_animacion_2d(SistemaOrbital *sistema)
{
    const int ancho_ventana = 900;              /* Ancho de la ventana en pixeles */
    const int alto_ventana = 620;               /* Alto de la ventana en pixeles */
    const double centro_px_x = ancho_ventana / 2.0;     /* Centro de la Tierra en X (px) */
    const double centro_px_y = alto_ventana / 2.0 + 30.0;   /* Centro en Y, corrido por el HUD */
    const double px_por_fila = (alto_ventana / 2.0 - 90.0) / 10.0;  /* Pixeles por fila de la matriz */

    double progreso_altura;                     /* Fraccion [0; 1] de la altura dentro del rango LEO */
    int radio_orb_filas;                        /* Radio de la orbita de referencia en filas (7 a 10) */
    double escala;                              /* Metros que representa cada fila */
    double px_por_metro;                        /* Conversion de metros a pixeles */

    int frames_total;                           /* Cantidad de cuadros de toda la animacion */
    double dt_sim;                              /* Tiempo simulado por cuadro (s) */
    double r;                                   /* Distancia actual al centro (m) */
    double vr = 0.0;                            /* Velocidad radial actual (m/s) */
    double nu = 0.0;                            /* Anomalia verdadera (rad) para la eliptica */
    int termino = 0;                            /* Bandera de fin de la animacion */
    int interrumpio = 0;                        /* Bandera: el usuario cerro la ventana o apreto ESC */
    int vuelta_actual = 1;                      /* Vuelta que se esta mostrando */

    Vector2 rastro[3000];                       /* Puntos ya recorridos por la nave */
    int cant_rastro = 0;                        /* Cantidad de puntos del rastro */
    Vector2 ruta[361];                          /* Trayectoria prevista completa (elipse/circulo) */
    int puntos_ruta = 0;                        /* Cantidad de puntos de la ruta prevista */
    Color color_nave;                           /* Color de la nave segun el estado */

    /* --- Escala visual: la orbita ocupa entre 7 y 10 filas segun la altura --- */
    progreso_altura = (sistema->altura_orbita - ALTURA_MIN) / (ALTURA_MAX - ALTURA_MIN);
    if (progreso_altura < 0.0) progreso_altura = 0.0;
    if (progreso_altura > 1.0) progreso_altura = 1.0;
    radio_orb_filas = 7 + (int)round(progreso_altura * 3.0);
    escala = sistema->radio_orbital / (double)radio_orb_filas;  /* Metros por fila */
    px_por_metro = px_por_fila / escala;                        /* Pixeles por metro */

    /* --- Ruta prevista: circulo (estable) o elipse (eliptica) --- */
    if (sistema->resultado_codigo == ESTADO_ESTABLE) {          /* Circunferencia de radio Ro */
        int k;                                                  /* Indice de muestreo del angulo */
        for (k = 0; k <= 360; k++) {                            /* 361 puntos, 1 por grado */
            double angulo = k * PI / 180.0;                     /* Angulo del muestreo */
            ruta[k].x = (float)(centro_px_x + sistema->radio_orbital * px_por_metro * cos(angulo));
            ruta[k].y = (float)(centro_px_y - sistema->radio_orbital * px_por_metro * sin(angulo));
        }
        puntos_ruta = 361;                                      /* Cantidad de puntos dibujados */
    } else if (sistema->resultado_codigo == ESTADO_ELIPTICA) {  /* Elipse por vis-viva y conica */
        double mu = G_UNIVERSAL * MASA_TIERRA;                  /* G*Mt */
        double eje_mayor = 1.0 / (2.0 / sistema->radio_orbital  /* 1/a = 2/Ro - v^2/mu */
             - (sistema->velocidad_orbital * sistema->velocidad_orbital) / mu);
        double excentricidad = 1.0 - sistema->radio_orbital / eje_mayor;    /* Perigeo en Ro */
        int k;                                                  /* Indice de muestreo de la anomalia */
        for (k = 0; k <= 360; k++) {                            /* 361 puntos sobre la conica */
            double angulo = k * PI / 180.0;                     /* Anomalia verdadera del muestreo */
            double radio = eje_mayor * (1.0 - excentricidad * excentricidad)
                           / (1.0 + excentricidad * cos(angulo));   /* r(nu) de la elipse */
            if (radio < 0.0 || radio > 10.0 * escala) {         /* Fuera del area visible */
                radio = 10.0 * escala;                          /* Recorta el dibujo al borde */
            }
            ruta[k].x = (float)(centro_px_x + radio * px_por_metro * cos(angulo));
            ruta[k].y = (float)(centro_px_y - radio * px_por_metro * sin(angulo));
        }
        puntos_ruta = 361;
    }

    preparar_tiempo(sistema, &frames_total, &dt_sim);           /* Tiempo acelerado de la animacion */
    r = sistema->radio_orbital;                                 /* La nave parte a distancia Ro */
    sistema->angulo_theta = 0.0;                                /* Reinicia el angulo */
    color_nave = color_estado(sistema->resultado_codigo);       /* Color segun el estado */

    /* --- Bucle principal de la ventana grafica --- */
    InitWindow(ancho_ventana, alto_ventana, "Simulador Artemis - Orbita LEO");  /* Crea la ventana */
    SetTargetFPS(1000 / FRAME_MS);                              /* ~12 cuadros por segundo */

    while (!WindowShouldClose() && !termino) {                  /* Hasta cerrar la ventana o terminar */
        int fila_equiv;                                         /* Posicion de la nave en filas (y) */
        int columna_equiv;                                      /* Posicion de la nave en columnas */
        double px, py;                                          /* Posicion de la nave en pixeles */

        if (IsKeyPressed(KEY_ESCAPE)) {                         /* ESC tambien interrumpe */
            termino = 1;
            interrumpio = 1;
        }

        fisica_cuadro(sistema, dt_sim, &r, &vr, &nu, &termino); /* Avanza el modelo un paso */

        /* Convierte la posicion fisica a pixeles (fila/columna equivalentes x2) */
        fila_equiv = (int)round(sistema->pos_y_real / escala);
        columna_equiv = (int)round(sistema->pos_x_real / escala * FACTOR_PROPORCION_X);
        px = centro_px_x + columna_equiv * px_por_fila;
        py = centro_px_y - fila_equiv * px_por_fila;

        /* Guarda el punto del rastro si esta dentro del cuadro y fuera de la Tierra */
        if (px > 0 && px < ancho_ventana && py > 40 && py < alto_ventana
            && fila_equiv * fila_equiv
               + (columna_equiv / FACTOR_PROPORCION_X) * (columna_equiv / FACTOR_PROPORCION_X)
               > RADIO_TIERRA_CONSOLA * RADIO_TIERRA_CONSOLA
            && cant_rastro < 3000) {
            rastro[cant_rastro].x = (float)px;                  /* Guarda el punto del recorrido */
            rastro[cant_rastro].y = (float)py;
            cant_rastro++;
        }

        /* --- Dibujo del cuadro --- */
        BeginDrawing();                                         /* Abre el modo dibujo de raylib */
        ClearBackground(RAYWHITE);                              /* Fondo blanco */

        /* HUD con los datos vivos de la simulacion */
        DrawText("SIMULADOR ARTEMIS - Orbita LEO", 20, 12, 22, DARKBLUE);
        DrawText(TextFormat("Estado: %s   Vuelta %d/%d", sistema->resultado_texto,
                            vuelta_actual, sistema->cantidad_vueltas), 20, 40, 18, color_nave);
        DrawText(TextFormat("Ro = %.0f km   v = %.1f m/s", sistema->radio_orbital / 1000.0,
                            sistema->velocidad_orbital), 20, 64, 16, DARKGRAY);
        DrawText(TextFormat("v ideal = %.1f m/s   v escape = %.1f m/s",
                            sistema->velocidad_ideal, sistema->velocidad_escape), 20, 84, 16, DARKGRAY);
        DrawText(TextFormat("theta = %.1f grados   omega = %.6f rad/s",
                            sistema->angulo_theta * 180.0 / PI, sistema->velocidad_angular),
                 20, 104, 16, DARKGRAY);

        /* Tierra: disco azul con borde y etiqueta */
        DrawCircle((int)centro_px_x, (int)centro_px_y,
                   (int)(RADIO_TIERRA_CONSOLA * px_por_fila), SKYBLUE);
        DrawCircleLines((int)centro_px_x, (int)centro_px_y,
                        (int)(RADIO_TIERRA_CONSOLA * px_por_fila), DARKBLUE);
        DrawText("Tierra", (int)centro_px_x - 22, (int)centro_px_y - 8, 16, DARKBLUE);

        /* Trayectoria prevista (linea fina y tenue) */
        if (puntos_ruta > 0) {
            DrawLineStrip(ruta, puntos_ruta, Fade(color_nave, 0.30f));
        }

        /* Rastro: puntos recorridos por la nave */
        {
            int i;                                              /* Indice del rastro */
            for (i = 0; i < cant_rastro; i++) {
                DrawCircleV(rastro[i], 2.5f, Fade(color_nave, 0.55f));
            }
        }

        /* Nave Orion: cuadradito (como el '#' del enunciado) con etiqueta */
        DrawRectangle((int)px - 5, (int)py - 5, 10, 10, color_nave);
        DrawCircleLines((int)px, (int)py, 9, DARKGRAY);
        DrawText("Orion", (int)px + 10, (int)py - 8, 14, DARKGRAY);

        /* Pie de la ventana con la ayuda para el usuario */
        DrawText("Cierre la ventana o presione ESC para interrumpir la animacion",
                 20, alto_ventana - 30, 16, GRAY);

        EndDrawing();                                           /* Cierra el modo dibujo y presenta */

        vuelta_actual = 1 + (int)(sistema->angulo_theta / (2.0 * PI));  /* Vuelta en curso */
        if (vuelta_actual > sistema->cantidad_vueltas) vuelta_actual = sistema->cantidad_vueltas;
    }

    /* --- Pantalla final con el desenlace (hasta cerrar, ENTER o ESC) --- */
    while (!WindowShouldClose() && !IsKeyPressed(KEY_ENTER) && !IsKeyPressed(KEY_ESCAPE)) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("SIMULADOR ARTEMIS - Resultado de la simulacion", 20, 12, 22, DARKBLUE);
        if (sistema->resultado_codigo == ESTADO_CAIDA) {
            DrawText(">>> IMPACTO: la nave reingreso a la atmosfera y toco la superficie.",
                     20, alto_ventana / 2 - 40, 20, RED);
        } else if (sistema->resultado_codigo == ESTADO_ESCAPE) {
            DrawText(">>> ESCAPE: la nave abandono la zona visible de la simulacion.",
                     20, alto_ventana / 2 - 40, 20, PURPLE);
        } else if (interrumpio) {
            DrawText(">>> Simulacion interrumpida por el usuario.",
                     20, alto_ventana / 2 - 40, 20, DARKGRAY);
        } else {
            DrawText(TextFormat(">>> Animacion completa: la nave recorrio %d vuelta(s) a la Tierra.",
                                sistema->cantidad_vueltas), 20, alto_ventana / 2 - 40, 20, GREEN);
        }
        DrawText("Presione ENTER o cierre la ventana para volver al menu",
                 20, alto_ventana / 2 + 10, 16, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();                                              /* Cierra la ventana grafica */
}

#else
/* ==================== VERSION DE CONSOLA (RESPALDO) ===================== */

/* Pauses de milisegundos y control del cursor solo en Windows/POSIX */
#ifdef _WIN32
#include <windows.h>    /* Para Sleep, SetConsoleCursorPosition y el cursor */
#else
#include <unistd.h>     /* Para usleep */
#endif

/* Pausa la ejecucion una cantidad de milisegundos sin consumir CPU de mas */
static void dormir_ms(int milisegundos)
{
#ifdef _WIN32
    Sleep((DWORD)milisegundos);                 /* Sleep recibe milisegundos */
#else
    usleep(milisegundos * 1000);                /* usleep recibe microsegundos */
#endif
}

/* Mueve el cursor del texto a la esquina superior izquierda (0,0) */
static void ir_a_origen(void)
{
#ifdef _WIN32
    COORD origen;                               /* Estructura {x, y} de Windows */
    origen.X = 0;
    origen.Y = 0;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), origen);
#else
    printf("\033[H");                           /* Codigo ANSI: cursor a casa */
#endif
}

/* Borra toda la pantalla antes de empezar la animacion en consola */
static void limpiar_pantalla(void)
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

/* Oculta el cursor parpadeante durante la animacion de consola */
static void ocultar_cursor(void)
{
#ifdef _WIN32
    HANDLE consola = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(consola, &info);
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consola, &info);
#endif
}

/* Vuelve a mostrar el cursor al terminar la animacion de consola */
static void mostrar_cursor(void)
{
#ifdef _WIN32
    HANDLE consola = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    GetConsoleCursorInfo(consola, &info);
    info.bVisible = TRUE;
    SetConsoleCursorInfo(consola, &info);
#endif
}

/* Escribe el contorno de la Tierra (circulo de RADIO_TIERRA_CONSOLA filas) */
static void dibujar_tierra(char matriz[ALTO_CONSOLA][ANCHO_CONSOLA + 2])
{
    int dy;                     /* Desplazamiento vertical respecto del centro */
    int dx;                     /* Desplazamiento horizontal respecto del centro */
    int medio_ancho;            /* Semiancho del circulo en columnas para cada fila */
    double distancia;           /* Distancia al centro en filas equivalentes */

    for (dy = -RADIO_TIERRA_CONSOLA; dy <= RADIO_TIERRA_CONSOLA; dy++) {
        medio_ancho = (int)round(sqrt((double)(RADIO_TIERRA_CONSOLA * RADIO_TIERRA_CONSOLA
                                               - dy * dy)) * FACTOR_PROPORCION_X);
        if (CENTRO_Y + dy < 0 || CENTRO_Y + dy >= ALTO_CONSOLA) {
            continue;                                                   /* Fuera de la matriz */
        }
        for (dx = -medio_ancho; dx <= medio_ancho; dx++) {
            distancia = sqrt((double)(dx * dx) / (FACTOR_PROPORCION_X * FACTOR_PROPORCION_X)
                             + (double)(dy * dy));
            if (fabs(distancia - (double)RADIO_TIERRA_CONSOLA) < 0.5) { /* Solo el borde */
                matriz[CENTRO_Y + dy][CENTRO_X + dx] = SIMBOLO_TIERRA;  /* Contorno 'O' */
            }
        }
    }
}

/* Convierte la posicion fisica (metros) a fila y columna de la matriz 61x25 */
static void convertir_a_consola(const SistemaOrbital *sistema, double escala,
                                int *fila, int *columna)
{
    *columna = CENTRO_X + (int)round(sistema->pos_x_real / escala * FACTOR_PROPORCION_X);
    *fila = CENTRO_Y - (int)round(sistema->pos_y_real / escala);
}

/* Dice si una celda pertenece al disco terrestre (para no borrar su contorno) */
static int celda_sobre_tierra(int fila, int columna)
{
    double dy = (double)(fila - CENTRO_Y);
    double dx = (double)(columna - CENTRO_X) / FACTOR_PROPORCION_X;
    return sqrt(dx * dx + dy * dy) <= (double)RADIO_TIERRA_CONSOLA + 0.5;
}

/* Imprime la matriz completa con un encabezado de datos de la simulacion */
static void imprimir_mundo(const char matriz[ALTO_CONSOLA][ANCHO_CONSOLA + 2],
                           const SistemaOrbital *sistema, int vuelta_actual)
{
    int fila;
    int columna;

    ir_a_origen();              /* Cursor arriba: el proximo cuadro sobreescribe */

    printf("  SIMULADOR ARTEMIS - Orbita LEO   Estado: %-8s  Vuelta %d/%d\n",
           sistema->resultado_texto, vuelta_actual, sistema->cantidad_vueltas);
    printf("  Ro = %.0f km   v = %.1f m/s   v ideal = %.1f m/s   v escape = %.1f m/s\n",
           sistema->radio_orbital / 1000.0, sistema->velocidad_orbital,
           sistema->velocidad_ideal, sistema->velocidad_escape);
    printf("  Nave '#': Orion   Tierra 'O': contorno   '.': trayectoria\n");

    for (fila = 0; fila < ALTO_CONSOLA; fila++) {
        putchar('|');
        for (columna = 0; columna < ANCHO_CONSOLA; columna++) {
            putchar(matriz[fila][columna]);
        }
        putchar('|');
        putchar('\n');
    }
    printf("  +-----------------------------------------------------------+\n");
    printf("  Angulo theta = %.1f grados   omega = %.6f rad/s\n",
           sistema->angulo_theta * 180.0 / PI, sistema->velocidad_angular);
}

/* Animacion en consola con la matriz de 61 x 25 caracteres */
void ejecutar_animacion_2d(SistemaOrbital *sistema)
{
    char matriz[ALTO_CONSOLA][ANCHO_CONSOLA + 2];   /* Grilla de caracteres */
    double progreso_altura;                         /* Fraccion de altura dentro del rango LEO */
    int radio_orb_filas;                            /* Radio de la orbita en filas (7 a 10) */
    double escala;                                  /* Metros por fila */
    int frames_total;                               /* Cuadros totales */
    double dt_sim;                                  /* Segundos simulados por cuadro */
    double r;                                       /* Radio actual (m) */
    double vr = 0.0;                                /* Velocidad radial (m/s) */
    double nu = 0.0;                                /* Anomalia verdadera (rad) */
    int fila, columna;                              /* Posicion de la nave en consola */
    int fila_anterior = -1, columna_anterior = -1;  /* Posicion del cuadro previo */
    int cuadro;                                     /* Indice del cuadro */
    int vuelta_actual;                              /* Vuelta en curso */
    int termino = 0;                                /* Bandera de fin */

    /* Escala visual: la orbita entre 7 y 10 filas segun la altura */
    progreso_altura = (sistema->altura_orbita - ALTURA_MIN) / (ALTURA_MAX - ALTURA_MIN);
    if (progreso_altura < 0.0) progreso_altura = 0.0;
    if (progreso_altura > 1.0) progreso_altura = 1.0;
    radio_orb_filas = 7 + (int)round(progreso_altura * 3.0);
    escala = sistema->radio_orbital / (double)radio_orb_filas;

    preparar_tiempo(sistema, &frames_total, &dt_sim);   /* Tiempo acelerado */
    r = sistema->radio_orbital;                         /* La nave parte en Ro */
    sistema->angulo_theta = 0.0;

    /* Cuadro inicial: matriz en blanco con la Tierra dibujada */
    memset(matriz, (int)SIMBOLO_VACIO, sizeof(matriz));
    for (fila = 0; fila < ALTO_CONSOLA; fila++) {
        matriz[fila][ANCHO_CONSOLA] = '\0';
    }
    dibujar_tierra(matriz);

    limpiar_pantalla();
    ocultar_cursor();

    for (cuadro = 0; cuadro < frames_total && !termino; cuadro++) { /* Bucle de cuadros */
        fisica_cuadro(sistema, dt_sim, &r, &vr, &nu, &termino);    /* Avanza el modelo */

        convertir_a_consola(sistema, escala, &fila, &columna);     /* A fila/columna */

        if (fila_anterior >= 0 && !celda_sobre_tierra(fila_anterior, columna_anterior)) {
            matriz[fila_anterior][columna_anterior] = SIMBOLO_RASTRO;  /* Deja el rastro '.' */
        }
        if (fila >= 0 && fila < ALTO_CONSOLA && columna >= 0 && columna < ANCHO_CONSOLA) {
            matriz[fila][columna] = SIMBOLO_NAVE;                      /* Dibuja la nave '#' */
        }
        fila_anterior = fila;
        columna_anterior = columna;

        vuelta_actual = 1 + (int)(sistema->angulo_theta / (2.0 * PI));
        if (vuelta_actual > sistema->cantidad_vueltas) vuelta_actual = sistema->cantidad_vueltas;

        imprimir_mundo(matriz, sistema, vuelta_actual);
        dormir_ms(FRAME_MS);
    }

    mostrar_cursor();
    ir_a_origen();
    printf("\n");
    if (sistema->resultado_codigo == ESTADO_CAIDA) {
        printf("  >>> IMPACTO: la nave reingreso a la atmosfera y toco la superficie.\n");
    } else if (sistema->resultado_codigo == ESTADO_ESCAPE) {
        printf("  >>> ESCAPE: la nave abandono la zona visible de la animacion.\n");
    } else {
        printf("  >>> Animacion completa: la nave recorrio %d vuelta(s) a la Tierra.\n",
               sistema->cantidad_vueltas);
    }
    dormir_ms(1200);
    limpiar_pantalla();
}

#endif /* USAR_RAYLIB */
