#include <stdio.h>
#include <stdlib.h>
#include "artemis.h"

/* Prototipos locales auxiliares para la navegacion del menu */
static void limpiar_pantalla(void);
static void pausar_consola(void);
static int leer_opcion_segura(int min, int max);
static void mostrar_banner_principal(void);
static void ejecutar_submenu_simulador(SistemaOrbital *sistema, HistorialSimulaciones *historial);

int main(void) {
    SistemaOrbital sistema;
    HistorialSimulaciones historial;
    int opcion_principal = 0;

    /* Inicializacion del estado general y del historial (tope = 20) */
    /* inicializar_historial(&historial); */
    /* cargar_parametros_estandar(&sistema); */

    do {
        limpiar_pantalla();
        mostrar_banner_principal();

        printf("\n  ======================================================================\n");
        printf("                             MENU DE OPCIONES\n");
        printf("  ======================================================================\n");
        printf("   [1] Breve descripcion del programa Artemis\n");
        printf("   [2] Descripcion del simulador\n");
        printf("   [3] Iniciar simulador\n");
        printf("   [4] Consultar historial de simulaciones\n");
        printf("   [5] Salir del simulador\n");
        printf("  ----------------------------------------------------------------------\n");
        printf("   Seleccione una opcion [1-5]: ");

        opcion_principal = leer_opcion_segura(1, 5);

        switch (opcion_principal) {
            case 1:
                limpiar_pantalla();
                printf("\n  [MODULO 1] Breve descripcion del programa Artemis\n");
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Pendiente de enlazar con mostrar_descripcion_artemis() en simulador.c)\n");
                /* mostrar_descripcion_artemis(); */
                pausar_consola();
                break;

            case 2:
                limpiar_pantalla();
                printf("\n  [MODULO 2] Descripcion del simulador y variables fisicas\n");
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Pendiente de enlazar con mostrar_descripcion_simulador() en simulador.c)\n");
                /* mostrar_descripcion_simulador(); */
                pausar_consola();
                break;

            case 3:
                /* Opcion 3: Core del programa (Submenu de simulacion) */
                ejecutar_submenu_simulador(&sistema, &historial);
                break;

            case 4:
                limpiar_pantalla();
                printf("\n  [MODULO 4] Historial de simulaciones (Max. %d registros)\n", MAX_HISTORIAL);
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Pendiente de enlazar con mostrar_historial_simulaciones() en simulador.c)\n");
                /* mostrar_historial_simulaciones(&historial); */
                pausar_consola();
                break;

            case 5:
                limpiar_pantalla();
                printf("\n  ======================================================================\n");
                printf("   Cerrando Artemis Simulator (Orbita Baja). Mision finalizada.\n");
                printf("  ======================================================================\n\n");
                break;
        }

    } while (opcion_principal != 5);

    return 0;
}

/* =========================================================================
 * SUBMENU DE LA OPCION 3 (CORE DEL PROGRAMA)
 * ========================================================================= */
static void ejecutar_submenu_simulador(SistemaOrbital *sistema, HistorialSimulaciones *historial) {
    int opcion_sub = 0;

    /* Evita advertencias del compilador mientras las llamadas estan comentadas */
    (void)sistema;
    (void)historial;

    do {
        limpiar_pantalla();
        mostrar_banner_principal();

        printf("\n  ======================================================================\n");
        printf("                      INICIAR SIMULADOR (OPCION 3)\n");
        printf("  ======================================================================\n");
        printf("   [1] Simulacion estandar (datos predefinidos por el sistema)\n");
        printf("   [2] Simulacion personalizada (modificar variables)\n");
        printf("   [3] Prueba de estabilidad (ingreso manual de velocidad inicial)\n");
        printf("   [4] Demostraciones de conceptos fisicos (preguntas con respuestas)\n");
        printf("   [5] Volver al menu principal\n");
        printf("  ----------------------------------------------------------------------\n");
        printf("   Seleccione un modo de simulacion [1-5]: ");

        opcion_sub = leer_opcion_segura(1, 5);

        switch (opcion_sub) {
            case 1:
                limpiar_pantalla();
                printf("\n  [MODO 3.1] Simulacion Estandar (h = %.0f m, Mn = %.0f kg)\n",
                       ALTURA_ORBITA_DEF, MASA_NAVE_DEF);
                printf("  ----------------------------------------------------------------------\n");
                printf("  Presione ENTER para comenzar la simulacion en 2D...");
                getchar();
                /*
                 * cargar_parametros_estandar(sistema);
                 * calcular_magnitudes_orbitales(sistema, 0);
                 * ejecutar_animacion_2d(sistema);
                 * guardar_en_historial(historial, sistema, "Estandar");
                 */
                pausar_consola();
                break;

            case 2:
                limpiar_pantalla();
                printf("\n  [MODO 3.2] Simulacion Personalizada (Ingreso validado de rangos)\n");
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Aqui se solicitaran Mn, h, vueltas, aceleracion a y tiempo t)\n");
                /*
                 * configurar_simulacion_personalizada(sistema);
                 * calcular_magnitudes_orbitales(sistema, 0);
                 * ejecutar_animacion_2d(sistema);
                 * guardar_en_historial(historial, sistema, "Personalizada");
                 */
                pausar_consola();
                break;

            case 3:
                limpiar_pantalla();
                printf("\n  [MODO 3.3] Prueba de Estabilidad (Rango [%.0f m/s; %.0f m/s])\n",
                       VEL_PRUEBA_MIN, VEL_PRUEBA_MAX);
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Aqui se evaluara si resulta en Orbita Estable, Caida, Eliptica o Escape)\n");
                /*
                 * configurar_prueba_estabilidad(sistema);
                 * calcular_magnitudes_orbitales(sistema, 1);
                 * ejecutar_animacion_2d(sistema);
                 * guardar_en_historial(historial, sistema, "Estabilidad");
                 */
                pausar_consola();
                break;

            case 4:
                limpiar_pantalla();
                printf("\n  [MODO 3.4] Demostraciones de Conceptos Fisicos (Preguntas del TP)\n");
                printf("  ----------------------------------------------------------------------\n");
                printf("  (Se ejecutara la simulacion demostrativa antes de mostrar los calculos)\n");
                /* ejecutar_demostraciones_conceptos(sistema, historial); */
                pausar_consola();
                break;

            case 5:
                /* Regresa al ciclo principal en main() */
                break;
        }

    } while (opcion_sub != 5);
}

/* =========================================================================
 * GRAFICO DEL TITULO Y DESARROLLADORES EN CONSOLA
 * ========================================================================= */
static void mostrar_banner_principal(void) {
    printf("\n");
    printf("                   +----------------------------------------+         \n");
    printf("                   |                                        |         \n");
    printf("                   |    Artemis Simulator (Orbita Baja)     |         \n");
    printf("                   |                                        |         \n");
    printf("                   +----------------------------------------+         \n");
    printf("\n");
    printf("  Desarrolladores: [Integrante 1] - [Integrante 2] - [Integrante 3] - [Integrante 4]\n");
    printf("  UTN FRRe | Algoritmos y Estructuras de Datos | Ingenieria en Sistemas\n");
}

/* =========================================================================
 * FUNCIONES AUXILIARES DE INTERFAZ Y VALIDACION TEMPORAL
 * (Luego se trasladaran al modulo validaciones.c / validaciones.h)
 * ========================================================================= */
static int leer_opcion_segura(int min, int max) {
    int opcion;
    int resultado_lectura;
    char caracter_residual;

    while (1) {
        resultado_lectura = scanf("%d", &opcion);

        /* Limpieza del buffer de entrada para evitar bucles infinitos con letras */
        while ((caracter_residual = getchar()) != '\n' && caracter_residual != EOF);

        if (resultado_lectura != 1) {
            printf("   [ERROR] Entrada invalida. Ingrese un numero entero [%d-%d]: ", min, max);
        } else if (opcion < min || opcion > max) {
            printf("   [ERROR] Opcion fuera de rango. Seleccione entre %d y %d: ", min, max);
        } else {
            return opcion;
        }
    }
}

static void limpiar_pantalla(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void pausar_consola(void) {
    printf("\n  Presione ENTER para volver al menu...");
    getchar();
}