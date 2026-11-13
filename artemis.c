#include <stdio.h>      /* Para snprintf (copiar texto con limite de tamano) */
#include <string.h>     /* Para memset (poner una estructura en cero) */
#include <math.h>       /* Para sqrt (raiz cuadrada) */
#include "artemis.h"    /* Constantes, estructuras y prototipos del modulo */
#include "validaciones.h"   /* Para usar las lecturas seguras por teclado */
#define GRAVEDAD_SUPERFICIE 9.80    /* g en la superficie terrestre (m/s^2), para comparar la aceleracion centripeta */
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

static void imprimir_separador(void)
{
    printf("  ----------------------------------------------------------------------\n");  
}

/* Pantalla 1 del menu principal: descripcion breve del programa Artemis */
void mostrar_descripcion_artemis(void)
{
    printf("\n  ======================================================================\n");   /* Linea decorativa superior */
    printf("                 PROGRAMA ARTEMIS (DESCRIPCION BREVE)\n");                        /* Titulo de la pantalla */
    printf("  ======================================================================\n");     /* Linea decorativa inferior */

    /* Parrafo 1: que es el programa Artemis */
    printf("   Artemis es el programa de exploracion espacial de la NASA cuyo\n");
    printf("   objetivo es volver a llevar seres humanos a la Luna y preparar\n");
    printf("   futuras misiones tripuladas mas lejanas.\n\n");

    /* Parrafo 2: los componentes principales de la mision */
    printf("   La mision utiliza el cohete Space Launch System (SLS) y la nave\n");
    printf("   Orion, que transporta a la tripulacion.\n\n");

    /* Parrafo 3: en que fase se centra este simulador */
    printf("   Tras el despegue, la nave es inyectada en una orbita terrestre\n");
    printf("   baja (LEO) de estacionamiento, antes de la inyeccion translunar.\n");
    printf("   Este simulador recrea esa fase: la insercion y la estabilidad\n");
    printf("   de la nave Orion en orbita LEO.\n");
    imprimir_separador();                                                                      /* Linea de cierre */
}

/* Pantalla 2 del menu principal: descripcion del simulador y sus reglas */
void mostrar_descripcion_simulador(void)
{
    printf("\n  ======================================================================\n");   /* Linea decorativa superior */
    printf("                     DESCRIPCION DEL SIMULADOR\n");                               /* Titulo de la pantalla */
    printf("  ======================================================================\n");     /* Linea decorativa inferior */

    /* Parrafo 1: que modela el simulador */
    printf("   Simula el movimiento circular uniforme (MCU) de la nave Orion\n");
    printf("   alrededor de la Tierra, donde la atraccion gravitatoria actua\n");
    printf("   como fuerza centripeta.\n\n");

    /* Parrafo 2: variables que el usuario puede cargar, con sus rangos reales (salen de las constantes) */
    printf("   Variables y rangos permitidos:\n");
    printf("    - Masa de la nave Mn ........ [%.0f ; %.0f] kg\n", MASA_NAVE_MIN, MASA_NAVE_MAX);
    printf("    - Altura orbital h .......... [%.0f ; %.0f] m\n", ALTURA_MIN, ALTURA_MAX);
    printf("    - Vueltas ................... [%d ; %d]\n", VUELTAS_MIN, VUELTAS_MAX);
    printf("    - Aceleracion lateral a ..... [%.0f ; %.0f] m/s^2\n", ACEL_LAT_MIN, ACEL_LAT_MAX);
    printf("    - Duracion del pulso t ...... [%.0f ; %.0f] s\n", TIEMPO_LAT_MIN, TIEMPO_LAT_MAX);
    printf("    - Velocidad (prueba) ........ [%.0f ; %.0f] m/s\n\n", VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);

    /* Parrafo 3: como se clasifica el resultado, segun la velocidad v y la ideal vi */
    printf("   Clasificacion de la orbita segun la velocidad v (vi = ideal):\n");
    printf("    - Caida ..... v < vi - %.0f m/s\n", TOLERANCIA_VEL);
    printf("    - Estable ... vi -/+ %.0f m/s\n", TOLERANCIA_VEL);
    printf("    - Eliptica .. vi + %.0f m/s <= v < velocidad de escape\n", TOLERANCIA_VEL);
    printf("    - Escape .... v >= velocidad de escape (raiz(2) * vi)\n");
    imprimir_separador();                                                                      /* Linea de cierre */
}

/* Respuesta 1: Fg actua como fuerza centripeta y velocidad orbital */
static void demo_fuerza_y_velocidad(const SistemaOrbital *s)
{
    double fc = s->masa_nave * s->velocidad_ideal * s->velocidad_ideal / s->radio_orbital;   /* Fc = Mn * v^2 / Ro con la v ideal */

    printf("\n  [1] La gravedad como fuerza centripeta y la velocidad orbital\n");   /* Titulo de la respuesta */
    imprimir_separador();                                                           /* Linea separadora */

    printf("   Fuerza gravitatoria:  Fg = G * Mt * Mn / Ro^2\n");                   /* Formula de Fg */
    printf("   Reemplazo:  Fg = %.3e * %.3e * %.0f / (%.0f)^2\n",
           G_UNIVERSAL, MASA_TIERRA, s->masa_nave, s->radio_orbital);               /* Valores usados */
    printf("   Resultado:  Fg = %.2f N\n\n", s->atraccion_gravitatoria);            /* Resultado de Fg */

    printf("   Fuerza centripeta:  Fc = Mn * v^2 / Ro\n");                          /* Formula de Fc */
    printf("   Reemplazo:  Fc = %.0f * (%.2f)^2 / %.0f\n",
           s->masa_nave, s->velocidad_ideal, s->radio_orbital);                     /* Valores usados */
    printf("   Resultado:  Fc = %.2f N\n\n", fc);                                   /* Resultado de Fc */

    printf("   Comprobacion:  |Fg - Fc| = %.4f N  ->  Fg actua como Fc\n\n",
           s->atraccion_gravitatoria > fc ? s->atraccion_gravitatoria - fc
                                          : fc - s->atraccion_gravitatoria);        /* Diferencia en valor absoluto */

    printf("   Al igualar Fg = Fc:  G*Mt*Mn / Ro^2 = Mn * v^2 / Ro\n");             /* Planteo de la igualdad */
    printf("   La masa Mn se cancela, por eso:  v = raiz(G * Mt / Ro)\n");          /* Despeje de la velocidad */
    printf("   Reemplazo:  v = raiz(%.3e * %.3e / %.0f)\n",
           G_UNIVERSAL, MASA_TIERRA, s->radio_orbital);                             /* Valores usados */
    printf("   Resultado:  v = %.2f m/s\n", s->velocidad_ideal);                    /* Velocidad orbital */
}

/* Respuesta 2: periodo orbital */
static void demo_periodo(const SistemaOrbital *s)
{
    printf("\n  [2] Periodo orbital (tiempo de una vuelta completa)\n");             /* Titulo de la respuesta */
    imprimir_separador();                                                           /* Linea separadora */

    printf("   Formula:  T = 2 * pi * Ro / v\n");                                   /* Formula del periodo */
    printf("   Reemplazo:  T = 2 * pi * %.0f / %.2f\n", s->radio_orbital, s->velocidad_ideal);   /* Valores usados */
    printf("   Resultado:  T = %.2f s = %.2f min\n", s->periodo_segundos, s->periodo_minutos);    /* En segundos y minutos */
    printf("   Para %d vuelta(s):  %.2f min\n",
           s->cantidad_vueltas, s->periodo_minutos * s->cantidad_vueltas);           /* Tiempo total de las vueltas pedidas */
}

/* Respuesta 3: aceleracion centripeta */
static void demo_centripeta(const SistemaOrbital *s)
{
    printf("\n  [3] Aceleracion centripeta\n");                                      /* Titulo de la respuesta */
    imprimir_separador();                                                           /* Linea separadora */

    printf("   Formula:  ac = v^2 / Ro   (equivale a G * Mt / Ro^2)\n");            /* Formula de ac */
    printf("   Reemplazo:  ac = (%.2f)^2 / %.0f\n", s->velocidad_ideal, s->radio_orbital);       /* Valores usados */
    printf("   Resultado:  ac = %.4f m/s^2\n", s->aceleracion_centripeta);          /* Resultado de ac */
    printf("   Comparacion con la gravedad en superficie:  ac = %.3f g\n",
           s->aceleracion_centripeta / GRAVEDAD_SUPERFICIE);                        /* ac expresada como multiplo de g */
}

/* Respuesta 4: pulso lateral, trayectoria resultante y desviacion */
static void demo_pulso_lateral(const SistemaOrbital *s)
{
    double v_final;     /* Modulo de la velocidad despues del pulso */
    double angulo;      /* Giro de la direccion de la velocidad, en grados */

    printf("\n  [4] Pulso de aceleracion lateral\n");                                /* Titulo de la respuesta */
    imprimir_separador();                                                           /* Linea separadora */

    if (s->aceleracion_lateral <= 0.0) {                                            /* Si no hay propulsion lateral */
        printf("   No hay pulso lateral (a = 0): la nave sigue en su orbita circular.\n");  /* Informa y termina */
        return;                                                                     /* No hay nada mas que calcular */
    }

    v_final = sqrt(s->velocidad_ideal * s->velocidad_ideal
                   + s->velocidad_lateral * s->velocidad_lateral);                  /* Composicion de velocidades perpendiculares */
    angulo = atan2(s->velocidad_lateral, s->velocidad_ideal) * 180.0 / PI;          /* Angulo de giro, pasado de radianes a grados */

    printf("   Velocidad transversal:  vlat = a * t\n");                            /* Formula de la velocidad lateral */
    printf("   Reemplazo:  vlat = %.2f * %.2f\n", s->aceleracion_lateral, s->tiempo_aceleracion);   /* Valores usados */
    printf("   Resultado:  vlat = %.2f m/s\n\n", s->velocidad_lateral);             /* Resultado de vlat */

    printf("   Desviacion lateral:  dy = 1/2 * a * t^2\n");                         /* Formula de la desviacion */
    printf("   Reemplazo:  dy = 0.5 * %.2f * (%.2f)^2\n", s->aceleracion_lateral, s->tiempo_aceleracion);   /* Valores usados */
    printf("   Resultado:  dy = %.3f m  (%.6f %% del radio orbital)\n\n",
           s->desviacion_lateral, 100.0 * s->desviacion_lateral / s->radio_orbital);  /* Resultado y su proporcion respecto de Ro */

    printf("   Trayectoria resultante:\n");                                         /* Descripcion cualitativa */
    printf("    - Durante el pulso la nave se desplaza %.3f m hacia el costado.\n", s->desviacion_lateral);
    printf("    - La velocidad pasa de %.4f a %.4f m/s (modulo casi igual).\n", s->velocidad_ideal, v_final);
    printf("    - Su direccion gira %.4f grados respecto de la original.\n", angulo);
    printf("    - Ya no es exactamente perpendicular al radio, por lo que la\n");
    printf("      orbita deja de ser circular perfecta: cambia levemente.\n");
}

/* Opcion 3.4: demostraciones de conceptos fisicos con preguntas y respuestas */
void ejecutar_demostraciones_conceptos(SistemaOrbital *sistema, HistorialSimulaciones *historial)
{
    int opcion;     /* Opcion elegida en el submenu de preguntas */

    printf("\n  === DEMOSTRACIONES DE CONCEPTOS FISICOS ===\n");                    /* Titulo de la pantalla */
    printf("   Desea usar los datos predefinidos del escenario? [S/N]: ");          /* Pregunta si usa datos fijos */

    if (leer_si_no()) {                                     /* Si respondio que si */
        cargar_parametros_estandar(sistema);                /* Carga los valores predefinidos */
    } else {                                                /* Si respondio que no */
        configurar_simulacion_personalizada(sistema);       /* Pide los datos por teclado */
    }

    calcular_magnitudes_orbitales(sistema, 0);              /* Calcula todas las magnitudes con la velocidad ideal */
    /* ejecutar_animacion_2d(sistema); */                   /* Cuando exista la parte grafica, se ejecuta aca antes de los calculos */
    guardar_en_historial(historial, sistema, "Demo");       /* Registra esta ejecucion en el historial */

    do {                                                    /* Repite el submenu hasta que elija volver */
        printf("\n  --- Preguntas de analisis (elija una) ---\n");   /* Titulo del submenu */
        printf("   [1] La gravedad como fuerza centripeta y la velocidad orbital\n");
        printf("   [2] Periodo orbital en segundos y minutos\n");
        printf("   [3] Aceleracion centripeta\n");
        printf("   [4] Pulso lateral: trayectoria y desviacion\n");
        printf("   [5] Volver\n");
        printf("   Seleccione [1-5]: ");                    /* Pide la opcion */
        opcion = leer_entero_rango(1, 5);                   /* Lee la opcion validada */

        switch (opcion) {                                   /* Ejecuta la respuesta elegida */
            case 1: demo_fuerza_y_velocidad(sistema); break;    /* Pregunta 1 */
            case 2: demo_periodo(sistema); break;               /* Pregunta 2 */
            case 3: demo_centripeta(sistema); break;            /* Pregunta 3 */
            case 4: demo_pulso_lateral(sistema); break;         /* Pregunta 4 */
            default: break;                                     /* La opcion 5 no hace nada: sale del bucle */
        }

        if (opcion != 5) {                                  /* Si mostro una respuesta */
            printf("\n  Presione ENTER para continuar...");      /* Pausa para poder leerla */
            esperar_enter();                                /* Espera el Enter del usuario */
        }
    } while (opcion != 5);                                  /* Termina al elegir volver */
}