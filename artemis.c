#include <stdio.h>      /* Para snprintf (copiar texto con limite de tamano) */
#include <string.h>     /* Para memset (poner una estructura en cero) */
#include <math.h>       /* Para sqrt (raiz cuadrada) */
#include "artemis.h"    /* Constantes, estructuras y prototipos del modulo */
#include "validaciones.h"   /* Para usar las lecturas seguras por teclado */

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

/* Modo personalizado: el usuario modifica masa, altura, vueltas y pulso lateral */
void configurar_simulacion_personalizada(SistemaOrbital *sistema)
{
    cargar_parametros_estandar(sistema);    /* Parte de los valores predefinidos y limpia el resto de los campos */

    printf("\n   --- Simulacion personalizada ---\n");   /* Titulo de la pantalla */

    printf("   Masa de la nave Mn [%.0f - %.0f] kg: ", MASA_NAVE_MIN, MASA_NAVE_MAX);   /* Pide la masa indicando el rango */
    sistema->masa_nave = leer_double_rango(MASA_NAVE_MIN, MASA_NAVE_MAX);              /* Guarda la masa validada */

    printf("   Altura orbital h [%.0f - %.0f] m: ", ALTURA_MIN, ALTURA_MAX);            /* Pide la altura indicando el rango */
    sistema->altura_orbita = leer_double_rango(ALTURA_MIN, ALTURA_MAX);                /* Guarda la altura validada */

    printf("   Cantidad de vueltas [%d - %d]: ", VUELTAS_MIN, VUELTAS_MAX);             /* Pide las vueltas indicando el rango */
    sistema->cantidad_vueltas = leer_entero_rango(VUELTAS_MIN, VUELTAS_MAX);           /* Guarda las vueltas validadas */

    leer_pulso_lateral(&sistema->aceleracion_lateral,   /* Pide a y t con la regla combinada; pasa la direccion de cada campo */
                       &sistema->tiempo_aceleracion,    /* para que la funcion los complete directamente en la estructura */
                       ACEL_LAT_MAX, TIEMPO_LAT_MAX);   /* Limites superiores permitidos */
}

/* Modo prueba de estabilidad: el usuario ingresa solo la velocidad inicial */
void configurar_prueba_estabilidad(SistemaOrbital *sistema)
{
    cargar_parametros_estandar(sistema);    /* Usa los valores predefinidos para todo lo demas */

    printf("\n   --- Prueba de estabilidad ---\n");   /* Titulo de la pantalla */
    printf("   Se usan: h = %.0f m y Mn = %.0f kg.\n", ALTURA_ORBITA_DEF, MASA_NAVE_DEF);  /* Informa los datos fijos */
    printf("   Velocidad inicial v [%.0f - %.0f] m/s: ", VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);  /* Pide la velocidad indicando el rango */
    sistema->velocidad_orbital = leer_double_rango(VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);        /* Guarda la velocidad validada */
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

/* Muestra el reporte con los datos de entrada y los resultados de una simulacion */
void mostrar_resultados_simulacion(const SistemaOrbital *sistema)
{
    printf("\n  ======================================================================\n"); 
    printf("                       REPORTE DE LA SIMULACION\n");                               
    printf("  ======================================================================\n");      

    printf("   DATOS DE ENTRADA\n");                                                            
    printf("   Masa de la nave Mn ........... %.2f kg\n", sistema->masa_nave);                  /* Masa ingresada o predefinida */
    printf("   Altura orbital h ............. %.2f m\n", sistema->altura_orbita);               /* Altura sobre la superficie */
    printf("   Vueltas a simular ............ %d\n", sistema->cantidad_vueltas);               /* Cantidad de vueltas */
    printf("   Pulso lateral: a = %.2f m/s^2 durante t = %.2f s\n",
           sistema->aceleracion_lateral, sistema->tiempo_aceleracion);                         /* Datos del propulsor auxiliar */

    printf("\n   MAGNITUDES ORBITALES (MCU)\n");                                              
    printf("   Radio orbital Ro = Rmt + h ... %.2f m\n", sistema->radio_orbital);              /* Radio desde el centro de la Tierra */
    printf("   Velocidad orbital ideal ...... %.2f m/s\n", sistema->velocidad_ideal);          /* v = raiz(G*Mt/Ro) */
    printf("   Velocidad de escape .......... %.2f m/s\n", sistema->velocidad_escape);         /* raiz(2) * v ideal */
    printf("   Velocidad utilizada .......... %.2f m/s\n", sistema->velocidad_orbital);        /* Velocidad con la que se evaluo la orbita */
    printf("   Fuerza gravitatoria Fg = Fc .. %.2f N\n", sistema->atraccion_gravitatoria);     /* Fg, que actua como fuerza centripeta */
    printf("   Aceleracion centripeta ac .... %.4f m/s^2\n", sistema->aceleracion_centripeta); /* ac = v^2 / Ro */
    printf("   Velocidad angular omega ...... %.6f rad/s\n", sistema->velocidad_angular);      /* omega = v / Ro */
    printf("   Periodo orbital T ............ %.2f s (%.2f min)\n",
           sistema->periodo_segundos, sistema->periodo_minutos);                               /* Tiempo de una vuelta completa */

    printf("\n   MANIOBRA LATERAL\n");                                                          /* Subtitulo de la seccion */
    printf("   Velocidad transversal ........ %.2f m/s\n", sistema->velocidad_lateral);        /* vlat = a * t */
    printf("   Desviacion lateral ........... %.3f m\n", sistema->desviacion_lateral);         /* dy = 1/2 * a * t^2 */

    printf("\n   RESULTADO: %s\n", sistema->resultado_texto);                                   /* Estable, Caida, Eliptica o Escape */
    printf("  ======================================================================\n");     /* Linea de cierre */
}

/* Guarda una simulacion en el historial. Si ya hay 20, descarta la mas antigua */
void guardar_en_historial(HistorialSimulaciones *historial, const SistemaOrbital *sistema, const char *modo)
{
    RegistroSimulacion *registro;   /* Puntero al registro donde se va a escribir */
    int i;                          /* Indice para el desplazamiento */

    if (historial->cantidad_simulaciones >= MAX_HISTORIAL) {        /* Si el historial esta lleno */
        for (i = 1; i < MAX_HISTORIAL; i++) {                       /* Recorre desde el segundo registro hasta el ultimo */
            historial->registros[i - 1] = historial->registros[i];  /* Copia cada registro a la posicion anterior (se pierde el mas antiguo) */
        }
        historial->cantidad_simulaciones = MAX_HISTORIAL - 1;       /* Deja libre la ultima posicion */
    }

    registro = &historial->registros[historial->cantidad_simulaciones];   /* Apunta al primer lugar libre */
    historial->total_ejecutadas++;                                  /* Suma una ejecucion al contador historico */

    registro->id_simulacion = historial->total_ejecutadas;          /* El ID es el numero correlativo de ejecucion */
    snprintf(registro->modo_simulacion, MAX_CADENA_ESTADO, "%s", modo);   /* Copia el nombre del modo sin pasarse del tamano */
    registro->masa_nave = sistema->masa_nave;                       /* Guarda la masa */
    registro->altura_orbita = sistema->altura_orbita;               /* Guarda la altura */
    registro->velocidad_orbital = sistema->velocidad_orbital;       /* Guarda la velocidad utilizada */
    registro->periodo_minutos = sistema->periodo_minutos;           /* Guarda el periodo en minutos */
    registro->aceleracion_lateral = sistema->aceleracion_lateral;   /* Guarda la aceleracion lateral */
    registro->tiempo_aceleracion = sistema->tiempo_aceleracion;     /* Guarda la duracion del pulso */
    registro->desviacion_lateral = sistema->desviacion_lateral;     /* Guarda la desviacion lateral */
    registro->cantidad_vueltas = sistema->cantidad_vueltas;         /* Guarda las vueltas */
    snprintf(registro->resultado, MAX_CADENA_ESTADO, "%s", sistema->resultado_texto);   /* Guarda el texto del resultado */

    historial->cantidad_simulaciones++;                             /* Ahora hay un registro mas ocupado */
}

/* Muestra el historial en forma de tabla */
void mostrar_historial_simulaciones(const HistorialSimulaciones *historial)
{
    int i;                          /* Indice para recorrer los registros */

    if (historial->cantidad_simulaciones == 0) {                    /* Si todavia no se ejecuto ninguna simulacion */
        printf("   No hay simulaciones registradas todavia.\n");    /* Avisa que esta vacio */
        return;                                                     /* Termina la funcion */
    }

    printf("   Simulaciones guardadas: %d de %d (ejecutadas en total: %d)\n\n",
           historial->cantidad_simulaciones, MAX_HISTORIAL, historial->total_ejecutadas);   /* Resumen de los contadores */

    printf("   %-3s %-13s %7s %8s %8s %7s %5s %5s %7s %3s %-9s\n",
           "ID", "Modo", "Mn[kg]", "h[km]", "v[m/s]", "T[min]", "a", "t[s]", "dy[m]", "Vlt", "Resultado");   /* Encabezados de las columnas */
    printf("   ------------------------------------------------------------------------------------\n");   /* Linea separadora */

    for (i = 0; i < historial->cantidad_simulaciones; i++) {        /* Recorre solo los registros ocupados */
        const RegistroSimulacion *r = &historial->registros[i];     /* Puntero de solo lectura al registro actual */
        printf("   %-3d %-13s %7.0f %8.1f %8.1f %7.2f %5.2f %5.1f %7.2f %3d %-9s\n",
               r->id_simulacion, r->modo_simulacion, r->masa_nave,
               r->altura_orbita / 1000.0,                           /* La altura se muestra en km para ahorrar ancho */
               r->velocidad_orbital, r->periodo_minutos, r->aceleracion_lateral,
               r->tiempo_aceleracion, r->desviacion_lateral,
               r->cantidad_vueltas, r->resultado);                  /* Una fila por simulacion */
    }
}