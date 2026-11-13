#include <stdio.h>          /* Para printf (todas las pantallas del menu) */
#include "artemis.h"        /* Constantes, estructuras y logica del simulador */
#include "validaciones.h"   /* Para leer_entero_rango (lectura segura del menu) */

/* Muestra el menu principal y devuelve la opcion elegida [1; 5] */
static int menu_principal(void)
{
    printf("  ==============================================================\n");
    printf("                    SIMULADOR ARTEMIS - ORBITA LEO              \n");
    printf("  ==============================================================\n");
    printf("     1. Breve descripcion del programa Artemis\n");
    printf("     2. Descripcion del simulador (objetivo, variables, fisica)\n");
    printf("     3. Iniciar simulador\n");
    printf("     4. Consultar historial de simulaciones\n");
    printf("     5. Salir\n");
    printf("  ==============================================================\n");
    printf("     Seleccione una opcion [1-5]: ");
    return leer_entero_rango(1, 5);         /* Repite hasta recibir un entero dentro del rango */
}

/* Ejecuta una corrida completa: calcula, anima el grafico y guarda en el
 * historial. usar_velocidad_manual en 1 respeta la velocidad ya cargada. */
static void ejecutar_corrida(SistemaOrbital *sistema, HistorialSimulaciones *historial, const char *modo)
{
    calcular_magnitudes_orbitales(sistema, sistema->velocidad_orbital > 0.0);   /* Motor de fisica */
    ejecutar_animacion_2d(sistema);                                             /* Grafico animado 61x25 */
    guardar_en_historial(historial, sistema, modo);                             /* Registro del historial */
    mostrar_resumen_simulacion(sistema);                                        /* Informe de magnitudes */
    pausar_enter();                                                             /* Pausa para leer el informe */
}

/* Submenu de la opcion 3: los cuatro modos del simulador */
static void submenu_simulador(HistorialSimulaciones *historial)
{
    SistemaOrbital sistema;         /* Estado completo de la simulacion en curso */
    int opcion;                     /* Opcion elegida en el submenu */

    do {                            /* Repite hasta que el usuario elija volver */
        printf("\n  ------------------ SIMULADOR (OPCION 3) ------------------\n");
        printf("     1. Simulacion estandar (datos del Problema 2)\n");
        printf("     2. Simulacion personalizada (ingreso y validacion de datos)\n");
        printf("     3. Prueba de estabilidad (velocidad manual)\n");
        printf("     4. Demostraciones de conceptos fisicos\n");
        printf("     5. Volver al menu principal\n");
        printf("     Seleccione una opcion [1-5]: ");
        opcion = leer_entero_rango(1, 5);   /* Lectura validada del submenu */

        switch (opcion) {           /* Cada modo prepara sus datos y ejecuta la corrida */
            case 1:                 /* Estandar: usa los valores predefinidos */
                cargar_parametros_estandar(&sistema);
                ejecutar_corrida(&sistema, historial, "Estandar");
                break;

            case 2:                 /* Personalizada: pide y valida cada dato */
                configurar_simulacion_personalizada(&sistema);
                ejecutar_corrida(&sistema, historial, "Personalizada");
                break;

            case 3:                 /* Estabilidad: pide la velocidad manual */
                configurar_prueba_estabilidad(&sistema);    /* Carga la velocidad elegida */
                ejecutar_corrida(&sistema, historial, "Estabilidad");
                break;

            case 4:                 /* Demos: submenu propio con animacion + teoria */
                ejecutar_demostraciones_conceptos(&sistema, historial);
                break;

            case 5:                 /* Volver: no hace nada, el do-while termina */
                break;
        }
    } while (opcion != 5);          /* Vuelve a mostrar el submenu tras cada modo */
}

int main(void)
{
    HistorialSimulaciones historial;    /* Contenedor de hasta 20 registros */
    int opcion;                         /* Opcion del menu principal */

    inicializar_historial(&historial);  /* Deja el historial vacio al comenzar */
    cargar_historial_archivo(&historial);   /* Recupera las simulaciones guardadas en historial.txt */
    mostrar_encabezado_creditos();      /* Pantalla de presentacion del grupo */

    do {                                /* Bucle principal del programa */
        opcion = menu_principal();      /* Muestra el menu y lee la opcion */

        switch (opcion) {               /* Ejecuta la opcion elegida */
            case 1:                     /* Descripcion breve de Artemis */
                mostrar_descripcion_artemis();
                break;
            case 2:                     /* Descripcion del simulador */
                mostrar_descripcion_simulador();
                break;
            case 3:                     /* Nucleo del programa: submenu del simulador */
                submenu_simulador(&historial);
                break;
            case 4:                     /* Tabla con el historial */
                mostrar_historial_simulaciones(&historial);
                break;
            case 5:                     /* Salir */
                printf("\n     Gracias por usar el Simulador Artemis. Hasta pronto!\n");
                break;
        }
    } while (opcion != 5);              /* Repite hasta que el usuario salga */

    return 0;                           /* Fin correcto del programa */
}
