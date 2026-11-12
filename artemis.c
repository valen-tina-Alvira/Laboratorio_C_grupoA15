#include <stdio.h>      /* Para snprintf (copiar texto con limite de tamano) */
#include <string.h>     /* Para memset (poner una estructura en cero) */
#include <math.h>       /* Para sqrt (raiz cuadrada) */
#include "artemis.h"    /* Constantes, estructuras y prototipos del modulo */

/* Convierte el codigo del estado orbital en su texto. static: solo se usa en este archivo */
static const char *texto_estado(EstadoOrbital estado)
{
    switch (estado) {                       /* Elige segun el valor del enum */
        case ESTADO_ESTABLE:  return "Estable";     /* Orbita circular estable */
        case ESTADO_CAIDA:    return "Caida";       /* Velocidad insuficiente */
        case ESTADO_ELIPTICA: return "Eliptica";    /* Velocidad mayor a la ideal, menor a la de escape */
        case ESTADO_ESCAPE:   return "Escape";      /* Velocidad de escape alcanzada */
        default:              return "Desconocido"; /* Por seguridad, si llega un valor inesperado */
    }
}

/* Deja el historial vacio: contadores en 0 y registros en 0 */
void inicializar_historial(HistorialSimulaciones *historial)
{
    memset(historial, 0, sizeof(*historial));   /* Pone en cero todos los bytes de la estructura apuntada */
}

/* Carga los valores predefinidos del escenario Artemis (modo estandar) */
void cargar_parametros_estandar(SistemaOrbital *sistema)
{
    memset(sistema, 0, sizeof(*sistema));               /* Limpia todos los campos para no dejar basura de una simulacion anterior */
    sistema->masa_nave = MASA_NAVE_DEF;                 /* Masa de la nave Orion: 33000 kg */
    sistema->altura_orbita = ALTURA_ORBITA_DEF;         /* Altura orbital: 192000 m */
    sistema->cantidad_vueltas = VUELTAS_DEF;            /* Vueltas por defecto: 1 */
    sistema->aceleracion_lateral = ACEL_PULSO_DEF;      /* Aceleracion lateral predefinida: 0.5 m/s^2 */
    sistema->tiempo_aceleracion = TIEMPO_PULSO_DEF;     /* Duracion del pulso predefinida: 10 s */
}

/* Calcula todas las magnitudes fisicas. Si usar_velocidad_manual es 1, respeta la velocidad que ya cargo el usuario */
void calcular_magnitudes_orbitales(SistemaOrbital *sistema, int usar_velocidad_manual)
{
    double mu = G_UNIVERSAL * MASA_TIERRA;      /* Producto G*Mt, que aparece en todas las formulas */
    double ro;                                  /* Radio orbital (copia local para escribir menos) */
    double v;                                   /* Velocidad orbital que se usa en los calculos */

    sistema->radio_orbital = RADIO_MEDIO_TIERRA + sistema->altura_orbita;   /* Ro = Rmt + h */
    ro = sistema->radio_orbital;                /* Guarda Ro en la variable local */

    sistema->velocidad_ideal = sqrt(mu / ro);           /* v ideal = raiz(G*Mt / Ro), de igualar Fg = Fc */
    sistema->velocidad_escape = sqrt(2.0 * mu / ro);    /* v escape = raiz(2*G*Mt / Ro), o sea raiz(2) * v ideal */
    sistema->limite_inferior_vel = sistema->velocidad_ideal - TOLERANCIA_VEL;  /* Por debajo de este valor: caida */
    sistema->limite_superior_vel = sistema->velocidad_ideal + TOLERANCIA_VEL;  /* Por encima de este valor: orbita eliptica */

    if (!usar_velocidad_manual) {                       /* Si no hay velocidad ingresada por el usuario */
        sistema->velocidad_orbital = sistema->velocidad_ideal;  /* Usa la velocidad ideal de la orbita circular */
    }
    v = sistema->velocidad_orbital;                     /* Guarda la velocidad activa en la variable local */

    sistema->atraccion_gravitatoria = mu * sistema->masa_nave / (ro * ro);  /* Fg = G*Mt*Mn / Ro^2 (en newtons) */
    sistema->aceleracion_centripeta = (v * v) / ro;     /* ac = v^2 / Ro (en m/s^2) */
    sistema->velocidad_angular = v / ro;                /* omega = v / Ro (en rad/s) */

    if (v > 0.0) {                                      /* Evita dividir por cero si la velocidad fuera nula */
        sistema->periodo_segundos = 2.0 * PI * ro / v;  /* T = 2*pi*Ro / v (en segundos) */
    } else {                                            /* Si la velocidad fuera nula */
        sistema->periodo_segundos = 0.0;                /* No hay periodo definido: queda en 0 */
    }
    sistema->periodo_minutos = sistema->periodo_segundos / 60.0;    /* Convierte el periodo a minutos */

    sistema->velocidad_lateral = sistema->aceleracion_lateral * sistema->tiempo_aceleracion;   /* vlat = a * t */
    sistema->desviacion_lateral = 0.5 * sistema->aceleracion_lateral
                                  * sistema->tiempo_aceleracion * sistema->tiempo_aceleracion; /* dy = 1/2 * a * t^2 */

    sistema->angulo_theta = 0.0;                        /* La nave parte del angulo 0 */
    sistema->pos_x_real = ro;                           /* Posicion inicial sobre el eje X: a distancia Ro del centro */
    sistema->pos_y_real = 0.0;                          /* Posicion inicial sobre el eje Y: 0 */

    evaluar_estabilidad_orbital(sistema);               /* Clasifica la orbita para que el resultado siempre quede actualizado */
}

/* Clasifica la orbita comparando la velocidad activa con los limites */
void evaluar_estabilidad_orbital(SistemaOrbital *sistema)
{
    double v = sistema->velocidad_orbital;      /* Velocidad que se va a evaluar */

    if (v < sistema->limite_inferior_vel) {             /* Menor que (v ideal - 50 m/s) */
        sistema->resultado_codigo = ESTADO_CAIDA;       /* La nave cae hacia la Tierra */
    } else if (v <= sistema->limite_superior_vel) {     /* Entre (v ideal - 50) y (v ideal + 50) */
        sistema->resultado_codigo = ESTADO_ESTABLE;     /* Orbita circular estable */
    } else if (v < sistema->velocidad_escape) {         /* Mayor que (v ideal + 50) pero menor que la de escape */
        sistema->resultado_codigo = ESTADO_ELIPTICA;    /* Orbita eliptica */
    } else {                                            /* Igual o mayor que la velocidad de escape */
        sistema->resultado_codigo = ESTADO_ESCAPE;      /* La nave abandona la atraccion terrestre */
    }

    snprintf(sistema->resultado_texto, MAX_CADENA_ESTADO, "%s",
             texto_estado(sistema->resultado_codigo));  /* Copia el nombre del estado en el campo de texto, sin pasarse del tamano */
}
