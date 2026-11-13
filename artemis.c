#include <stdio.h>      /* Para snprintf (copiar texto con limite de tamano) */
#include <string.h>     /* Para memset (poner una estructura en cero) */
#include <math.h>       /* Para sqrt (raiz cuadrada) */
#include "artemis.h"    /* Constantes, estructuras y prototipos del modulo */
#include "validaciones.h"   /* Para usar las lecturas seguras por teclado */

/* =========================================================================
 * 1. CONSTANTES LOCALES
 * ========================================================================= */
#define GRAVEDAD_SUPERFICIE 9.80    /* g en la superficie terrestre (m/s^2), para comparar la aceleracion centripeta */

/* =========================================================================
 * 2. FUNCIONES AUXILIARES INTERNAS
 * ========================================================================= */
/* Convierte el codigo del estado orbital en su texto. static: solo se usa en este archivo */
static const char *texto_estado(EstadoOrbital estado)
{
    switch (estado) {                       /* Elige segun el valor del enum */
        case ESTADO_ESTABLE:  return "Estable";     
        case ESTADO_CAIDA:    return "Caida";       
        case ESTADO_ELIPTICA: return "Eliptica";    
        case ESTADO_ESCAPE:   return "Escape";    
        default:              return "Desconocido"; 
    }
}
static void imprimir_separador(void)
{
    printf("  ----------------------------------------------------------------------\n");  
}

/* =========================================================================
 * 3. INICIALIZACION Y CONFIGURACION DE PARAMETROS
 * ========================================================================= */

void inicializar_historial(HistorialSimulaciones *historial)
{
    memset(historial, 0, sizeof(*historial));    /* Contadores y registros en cero */
}

/* Carga los valores predefinidos del escenario Artemis (modo estandar) */
void cargar_parametros_estandar(SistemaOrbital *sistema)
{
    memset(sistema, 0, sizeof(*sistema));     /* Limpia tambien los resultados de una corrida anterior */         
    sistema->masa_nave = MASA_NAVE_DEF;               
    sistema->altura_orbita = ALTURA_ORBITA_DEF;        
    sistema->cantidad_vueltas = VUELTAS_DEF;            
    sistema->aceleracion_lateral = ACEL_PULSO_DEF;     
    sistema->tiempo_aceleracion = TIEMPO_PULSO_DEF;     
}

/* Modo personalizado: el usuario modifica masa, altura, vueltas y pulso lateral */
void configurar_simulacion_personalizada(SistemaOrbital *sistema)
{
    cargar_parametros_estandar(sistema);   

    printf("\n   --- Simulacion personalizada ---\n");   /* Titulo de la pantalla */

    printf(" ingrese el valor de la Masa de la nave Mn [%.0f - %.0f] kg: ", MASA_NAVE_MIN, MASA_NAVE_MAX);   
    sistema->masa_nave = leer_double_rango(MASA_NAVE_MIN, MASA_NAVE_MAX);              /* Guarda la masa validada */

    printf(" ingrese el valor de la Altura orbital h [%.0f - %.0f] m: ", ALTURA_MIN, ALTURA_MAX);  
    sistema->altura_orbita = leer_double_rango(ALTURA_MIN, ALTURA_MAX);                

    printf(" ingrese el valor de la Cantidad de vueltas [%d - %d]: ", VUELTAS_MIN, VUELTAS_MAX);             
    sistema->cantidad_vueltas = leer_entero_rango(VUELTAS_MIN, VUELTAS_MAX);           

     /* Se pasan las direcciones de los campos para que la funcion los complete directamente */
    leer_pulso_lateral(&sistema->aceleracion_lateral, &sistema->tiempo_aceleracion,    
                         ACEL_LAT_MAX, TIEMPO_LAT_MAX);   /* Limites superiores permitidos */
}

/* Modo prueba de estabilidad: el usuario ingresa solo la velocidad inicial */
void configurar_prueba_estabilidad(SistemaOrbital *sistema)
{
    cargar_parametros_estandar(sistema); 

    printf("\n   --- Prueba de estabilidad ---\n");   
    printf("   Se usan: h = %.0f m y Mn = %.0f kg.\n", ALTURA_ORBITA_DEF, MASA_NAVE_DEF);  
    printf("   Velocidad inicial v [%.0f - %.0f] m/s: ", VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);  /* Pide la velocidad indicando el rango */
    sistema->velocidad_orbital = leer_double_rango(VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);        /* Guarda la velocidad validada */
}

/* =========================================================================
 * 4. MOTOR DE CALCULOS FISICOS Y EVALUACION ORBITAL
 * ========================================================================= */

/* Calcula todas las magnitudes fisicas.
 Si usar_velocidad_manual es 1, respeta la velocidad que ya cargo el usuario;
 con 0 usa la velocidad ideal. */
void calcular_magnitudes_orbitales(SistemaOrbital *sistema, int usar_velocidad_manual)
{
    double mu = G_UNIVERSAL * MASA_TIERRA;      /* Producto G*Mt, que aparece en todas las formulas */
    double ro;                                  /* Radio orbital */
    double v;                                   /* Velocidad orbital */

    sistema->radio_orbital = RADIO_MEDIO_TIERRA + sistema->altura_orbita;   /* Ro = Rmt + h */
    ro = sistema->radio_orbital;                /* Guarda Ro en la variable local */

    /* Igualar Fg = Fc: G*Mt*Mn/Ro^2 = Mn*v^2/Ro. La masa de la nave se cancela. */
    sistema->velocidad_ideal = sqrt(mu / ro);           
    sistema->velocidad_escape = sqrt(2.0 * mu / ro);    
    sistema->limite_inferior_vel = sistema->velocidad_ideal - TOLERANCIA_VEL;  /* Por debajo de este valor: caida */
    sistema->limite_superior_vel = sistema->velocidad_ideal + TOLERANCIA_VEL;  /* Por encima de este valor: orbita eliptica */

    /* Si no hay velocidad ingresada por el usuario; Usa la velocidad ideal de la orbita circular */
    if (!usar_velocidad_manual) {    
        sistema->velocidad_orbital = sistema->velocidad_ideal;  
    }
    v = sistema->velocidad_orbital;

    sistema->atraccion_gravitatoria = mu * sistema->masa_nave / (ro * ro);
    sistema->aceleracion_centripeta = (v * v) / ro;     /* ac = v^2 / Ro (en m/s^2) */
    sistema->velocidad_angular = v / ro;                /* omega = v / Ro (en rad/s) */

    if (v > 0.0) {                                      
        sistema->periodo_segundos = 2.0 * PI * ro / v;  /* T = 2*pi*Ro / v (en segundos) */
    } else {                                            /* Si la velocidad fuera nula */
        sistema->periodo_segundos = 0.0;                /* No hay periodo definido: queda en 0 */
    }
    sistema->periodo_minutos = sistema->periodo_segundos / 60.0;    /* Convierte el periodo a minutos */

     /* Pulso lateral: vlat = a*t y dy = 1/2*a*t^2 */
    sistema->velocidad_lateral = sistema->aceleracion_lateral * sistema->tiempo_aceleracion;   /* vlat = a * t */
    sistema->desviacion_lateral = 0.5 * sistema->aceleracion_lateral
                                  * sistema->tiempo_aceleracion * sistema->tiempo_aceleracion; /* dy = 1/2 * a * t^2 */

    /* Posicion inicial de la nave para la animacion */
    sistema->angulo_theta = 0.0;                       
    sistema->pos_x_real = ro;                           
    sistema->pos_y_real = 0.0;                          

    evaluar_estabilidad_orbital(sistema);  /* Clasifica la orbita para que el resultado siempre quede actualizado */
}

/* Clasifica la orbita comparando la velocidad activa con los limites */
void evaluar_estabilidad_orbital(SistemaOrbital *sistema)
{
    double v = sistema->velocidad_orbital;     

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
             texto_estado(sistema->resultado_codigo));  
}

/* =========================================================================
 * 5. HISTORIAL DE SIMULACIONES
 * ========================================================================= */
/* Guarda una simulacion en el historial. Si ya hay 20, descarta la mas antigua */
void guardar_en_historial(HistorialSimulaciones *historial, const SistemaOrbital *sistema, const char *modo)
{
    RegistroSimulacion *registro;   
    int i;                          

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
    registro->masa_nave = sistema->masa_nave;                       
    registro->altura_orbita = sistema->altura_orbita;               
    registro->velocidad_orbital = sistema->velocidad_orbital;       
    registro->periodo_minutos = sistema->periodo_minutos;           
    registro->aceleracion_lateral = sistema->aceleracion_lateral;   
    registro->tiempo_aceleracion = sistema->tiempo_aceleracion;     
    registro->desviacion_lateral = sistema->desviacion_lateral;     
    registro->cantidad_vueltas = sistema->cantidad_vueltas;       
    snprintf(registro->resultado, MAX_CADENA_ESTADO, "%s", sistema->resultado_texto);   

    historial->cantidad_simulaciones++;                             /* Ahora hay un registro mas ocupado */
}

/* Muestra el historial en forma de tabla */
void mostrar_historial_simulaciones(const HistorialSimulaciones *historial)
{
    int i;                         

    if (historial->cantidad_simulaciones == 0) {                    /* Si todavia no se ejecuto ninguna simulacion, avisa que esta vacio  */
        printf("   No hay simulaciones registradas todavia.\n");   
        return;                                                     
    }

    printf("   Simulaciones guardadas: %d de %d (ejecutadas en total: %d)\n\n",
           historial->cantidad_simulaciones, MAX_HISTORIAL, historial->total_ejecutadas);   /* Resumen de los contadores */

    printf("   %-3s %-13s %7s %8s %8s %7s %5s %5s %7s %3s %-9s\n",
           "ID", "Modo", "Mn[kg]", "h[km]", "v[m/s]", "T[min]", "a", "t[s]", "dy[m]", "Vlt", "Resultado");   /* Encabezados de las columnas */
    printf("   ------------------------------------------------------------------------------------\n");   

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

/* =========================================================================
 * 6. REPORTES Y PANTALLAS INFORMATIVAS
 * ========================================================================= */

 /* Muestra el reporte con los datos de entrada y los resultados de una simulacion */
void mostrar_resumen_simulacion(const SistemaOrbital *sistema)
{
    printf("\n  ======================================================================\n"); 
    printf("                       REPORTE DE LA SIMULACION\n");                               
    printf("  ======================================================================\n");      

    printf("   DATOS DE ENTRADA\n");                                                            
    printf("   Masa de la nave Mn ........... %.2f kg\n", sistema->masa_nave);                  
    printf("   Altura orbital h ............. %.2f m\n", sistema->altura_orbita);               
    printf("   Vueltas a simular ............ %d\n", sistema->cantidad_vueltas);               
    printf("   Pulso lateral: a = %.2f m/s^2 durante t = %.2f s\n",
           sistema->aceleracion_lateral, sistema->tiempo_aceleracion);                         

    printf("\n   MAGNITUDES ORBITALES (MCU)\n");                                              
    printf("   Radio orbital Ro = Rmt + h ... %.2f m\n", sistema->radio_orbital);              
    printf("   Velocidad orbital ideal ...... %.2f m/s\n", sistema->velocidad_ideal);          
    printf("   Velocidad de escape .......... %.2f m/s\n", sistema->velocidad_escape);         
    printf("   Velocidad utilizada .......... %.2f m/s\n", sistema->velocidad_orbital);        
    printf("   Fuerza gravitatoria Fg = Fc .. %.2f N\n", sistema->atraccion_gravitatoria);     
    printf("   Aceleracion centripeta ac .... %.4f m/s^2\n", sistema->aceleracion_centripeta); 
    printf("   Velocidad angular omega ...... %.6f rad/s\n", sistema->velocidad_angular);      
    printf("   Periodo orbital T ............ %.2f s (%.2f min)\n",
           sistema->periodo_segundos, sistema->periodo_minutos);                               

    printf("\n   MANIOBRA LATERAL\n");                                                          
    printf("   Velocidad transversal ........ %.2f m/s\n", sistema->velocidad_lateral);       
    printf("   Desviacion lateral ........... %.3f m\n", sistema->desviacion_lateral);        

    printf("\n   RESULTADO: %s\n", sistema->resultado_texto);                                   /* Estable, Caida, Eliptica o Escape */
    printf("  ======================================================================\n");    
}

/* Pantalla de presentacion que se muestra una sola vez al abrir el programa */
void mostrar_encabezado_creditos(void)
{
    printf("\n");
    printf("  ======================================================================\n");   
    printf("                 ARTEMIS SIMULATOR (ORBITA TERRESTRE BAJA - LEO)\n");         
    printf("  ======================================================================\n");   
    printf("   Desarrolladores: [Segovia, Sebastian] - [Alvira Coseres, Valentina] - [Alfonso, valentina] - [Saade, antonio]\n");  
    printf("   UTN FRRe | Algoritmos y Estructuras de Datos | Ingenieria en Sistemas\n");   
    printf("   Situacion-Problema 2: Insercion y Estabilidad en Orbita LEO (MCU y Gravitacion)\n");   
    printf("  ======================================================================\n\n"); 
}

/* Pantalla 1 del menu principal: descripcion breve del programa Artemis */
void mostrar_descripcion_artemis(void)
{
    printf("\n  ======================================================================\n");  
    printf("                 PROGRAMA ARTEMIS (DESCRIPCION BREVE)\n");                        
    printf("  ======================================================================\n");     

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
    imprimir_separador();  
}

/* Pantalla 2 del menu principal: descripcion del simulador y sus reglas */
void mostrar_descripcion_simulador(void)
{
    printf("\n  ======================================================================\n");   
    printf("                     DESCRIPCION DEL SIMULADOR\n");                               
    printf("  ======================================================================\n");    

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
    printf("    - Estable ... vi - %.0f <= v <= vi + %.0f m/s\n", TOLERANCIA_VEL, TOLERANCIA_VEL);   /* Rango estable, ambos bordes incluidos */
    printf("    - Eliptica .. vi + %.0f < v < velocidad de escape\n", TOLERANCIA_VEL);               /* Desde justo por encima del limite superior */
    printf("    - Escape .... v >= velocidad de escape (raiz(2) * vi)\n");
    imprimir_separador();                                                                      
}

/* =========================================================================
 * 7. DEMOSTRACIONES DE CONCEPTOS FISICOS (OPCION 3.4)
 * ========================================================================= */
/* Respuesta 1: Fg actua como fuerza centripeta y velocidad orbital */
static void demo_fuerza_y_velocidad(const SistemaOrbital *s)
{
    double fc = s->masa_nave * s->velocidad_ideal * s->velocidad_ideal / s->radio_orbital;   /* Fc = Mn * v^2 / Ro con la v ideal */

    printf("\n  [1] La gravedad como fuerza centripeta y la velocidad orbital\n");   
    imprimir_separador();                                  

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
    printf("\n  [2] Periodo orbital (tiempo de una vuelta completa)\n");             
    imprimir_separador();                                                           

    printf("   Formula:  T = 2 * pi * Ro / v\n");                                   /* Formula del periodo */
    printf("   Reemplazo:  T = 2 * pi * %.0f / %.2f\n", s->radio_orbital, s->velocidad_ideal);   /* Valores usados */
    printf("   Resultado:  T = %.2f s = %.2f min\n", s->periodo_segundos, s->periodo_minutos);    /* En segundos y minutos */
    printf("   Para %d vuelta(s):  %.2f min\n",
           s->cantidad_vueltas, s->periodo_minutos * s->cantidad_vueltas);           /* Tiempo total de las vueltas pedidas */
}

/* Respuesta 3: aceleracion centripeta */
static void demo_centripeta(const SistemaOrbital *s)
{
    printf("\n  [3] Aceleracion centripeta\n");                                      
    imprimir_separador();                                                           

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
    /* La velocidad lateral es perpendicular a la orbital: se componen con Pitagoras */
    v_final = sqrt(s->velocidad_ideal * s->velocidad_ideal
                   + s->velocidad_lateral * s->velocidad_lateral);                  /* Composicion de velocidades perpendiculares */
    angulo = atan2(s->velocidad_lateral, s->velocidad_ideal) * 180.0 / PI;          /* Angulo de giro, pasado de radianes a grados */

    printf("   Velocidad transversal:  vlat = a * t\n");                            
    printf("   Reemplazo:  vlat = %.2f * %.2f\n", s->aceleracion_lateral, s->tiempo_aceleracion);   
    printf("   Resultado:  vlat = %.2f m/s\n\n", s->velocidad_lateral);            

    printf("   Desviacion lateral:  dy = 1/2 * a * t^2\n");                         
    printf("   Reemplazo:  dy = 0.5 * %.2f * (%.2f)^2\n", s->aceleracion_lateral, s->tiempo_aceleracion);  
    printf("   Resultado:  dy = %.3f m  (%.6f %% del radio orbital)\n\n",
           s->desviacion_lateral, 100.0 * s->desviacion_lateral / s->radio_orbital);

    printf("   Trayectoria resultante:\n");                                         /* Descripcion */
    printf("    - Durante el pulso la nave se desplaza %.3f m hacia el costado.\n", s->desviacion_lateral);
    printf("    - La velocidad pasa de %.4f a %.4f m/s (modulo casi igual).\n", s->velocidad_ideal, v_final);
    printf("    - Su direccion gira %.4f grados respecto de la original.\n", angulo);
    printf("    - Ya no es exactamente perpendicular al radio, por lo que la\n");
    printf("      orbita deja de ser circular perfecta: cambia levemente.\n");
}

/* Opcion 3.4: elige datos, muestra la animacion y despues las respuestas teoricas */
void ejecutar_demostraciones_conceptos(SistemaOrbital *sistema, HistorialSimulaciones *historial)
{
    int opcion;   

    printf("\n  === DEMOSTRACIONES DE CONCEPTOS FISICOS ===\n");                    
    printf("   Desea usar los datos predefinidos del escenario? [S/N]: ");         

    if (leer_si_no()) {                                     /* Si respondio que si Carga los valores predefinidos  */
        cargar_parametros_estandar(sistema);              
    } else {                                                /* Si respondio que no  Pide los datos por teclado */
        configurar_simulacion_personalizada(sistema);      
    }

    calcular_magnitudes_orbitales(sistema, 0);             
    ejecutar_animacion_2d(sistema);                  
    guardar_en_historial(historial, sistema, "Demo");       

    do {                                                    /* Repite el submenu hasta que elija volver */
        printf("\n  --- Preguntas de analisis (elija una) ---\n");   
        printf("   [1] La gravedad como fuerza centripeta y la velocidad orbital\n");
        printf("   [2] Periodo orbital en segundos y minutos\n");
        printf("   [3] Aceleracion centripeta\n");
        printf("   [4] Pulso lateral: trayectoria y desviacion\n");
        printf("   [5] Volver\n");
        printf("   Seleccione [1-5]: ");                    
        opcion = leer_entero_rango(1, 5);                   

        switch (opcion) {                                   /* Ejecuta la respuesta elegida */
            case 1: demo_fuerza_y_velocidad(sistema); break;   
            case 2: demo_periodo(sistema); break;              
            case 3: demo_centripeta(sistema); break;            
            case 4: demo_pulso_lateral(sistema); break;         
            default: break;                                     /* La opcion 5 sale del bucle */
        }

        if (opcion != 5) {                                  /* Si mostro una respuesta */
            printf("\n  Presione ENTER para continuar...");      /* Pausa para poder leerla */
            pausar_enter ();                                
        }
    } while (opcion != 5);                                  /* Termina al elegir volver */
}
