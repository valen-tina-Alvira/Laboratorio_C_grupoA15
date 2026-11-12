
ifndef ARTEMIS_H                   /* Si ARTEMIS_H no esta definido todavia, incluye este archivo */
#define ARTEMIS_H                   /* Lo define para que el archivo no se incluya dos veces (guard) */

/* =========================================================================
 * 1. CONSTANTES FISICAS UNIVERSALES Y MATEMATICAS
 * ========================================================================= */
#define G_UNIVERSAL 6.674e-11       /* Constante de gravitacion universal (N*m^2/kg^2) */
#define MASA_TIERRA 5.972e24        /* Masa de la Tierra Mt (kg) */
#define RADIO_MEDIO_TIERRA 6371000.0 /* Radio medio terrestre Rmt (m) */
#define PI 3.14159265358979323846   /* Constante matematica Pi */

/* =========================================================================
 * 2. VALORES PREDEFINIDOS DEL ESCENARIO ARTEMIS (PROBLEMA 2 - LEO)
 * ========================================================================= */
#define MASA_NAVE_DEF 33000.0       /* Masa predefinida de la nave Orion Mn (kg) */
#define ALTURA_ORBITA_DEF 192000.0  /* Altura orbital predefinida h (m) */
#define RADIO_ORBITAL_DEF 6563000.0 /* Radio orbital base Ro = Rmt + h (m) */
#define VEL_IDEAL_DEF 7792.93       /* Velocidad orbital ideal base para h = 192 km (m/s) */
#define VEL_ESCAPE_DEF 11020.87     /* Velocidad de escape base para h = 192 km (m/s) */
#define TOLERANCIA_VEL 50.0         /* Margen de tolerancia para orbita circular (m/s) */
#define ACEL_PULSO_DEF 0.50         /* Aceleracion lateral predefinida para demostracion (m/s^2) */
#define TIEMPO_PULSO_DEF 10.0       /* Duracion del pulso predefinida para demostracion (s) */
#define VUELTAS_DEF 1               /* Cantidad de vueltas por defecto */

/* =========================================================================
 * 3. RANGOS Y LIMITES DE VALIDACION DE ENTRADA
 * ========================================================================= */
#define MASA_NAVE_MIN 10000.0       /* Masa minima permitida para la nave (kg) */
#define MASA_NAVE_MAX 50000.0       /* Masa maxima permitida para la nave (kg) */

#define ALTURA_MIN 160000.0         /* Altura minima de orbita terrestre baja LEO (m) */
#define ALTURA_MAX 2000000.0        /* Altura maxima de orbita terrestre baja LEO (m) */

#define VEL_PRUEBA_MIN 6500.0       /* Velocidad manual minima en prueba de estabilidad (m/s) */
#define VEL_PRUEBA_MAX 11200.0      /* Velocidad manual maxima en prueba de estabilidad (m/s) */

#define ACEL_LAT_MIN 0.0            /* Aceleracion lateral minima (m/s^2) */
#define ACEL_LAT_MAX 15.0           /* Aceleracion lateral maxima (m/s^2) */

#define TIEMPO_LAT_MIN 0.0          /* Duracion minima del pulso lateral (s) */
#define TIEMPO_LAT_MAX 60.0         /* Duracion maxima del pulso lateral (s) */

#define VUELTAS_MIN 1               /* Cantidad minima de vueltas a la Tierra */
#define VUELTAS_MAX 10              /* Cantidad maxima de vueltas a la Tierra */

/* =========================================================================
 * 4. CONSTANTES DE DIMENSION EN CONSOLA Y SISTEMA
 * ========================================================================= */
#define ANCHO_CONSOLA 61            /* Columnas de la matriz 2D */
#define ALTO_CONSOLA 25             /* Filas de la matriz 2D */
#define CENTRO_X 30                 /* Columna central para ubicar el polo terrestre */
#define CENTRO_Y 12                 /* Fila central para ubicar el polo terrestre */
#define FACTOR_PROPORCION_X 2.0     /* Compensacion de aspecto horizontal en consola */
#define RADIO_TIERRA_CONSOLA 5      /* Filas de radio para dibujar la Tierra */
#define RADIO_ORBITA_CONSOLA 9      /* Filas de radio base para dibujar la orbita */

#define SIMBOLO_NAVE '#'            /* Caracter que representa el cubo de la nave Orion */
#define SIMBOLO_TIERRA 'O'          /* Caracter para el contorno de la Tierra */
#define SIMBOLO_RASTRO '.'          /* Caracter para la linea de puntos de la trayectoria */
#define SIMBOLO_VACIO ' '           /* Caracter para el espacio vacio de la grilla */

#define MAX_HISTORIAL 20            /* Capacidad maxima del arreglo de simulaciones */
#define MAX_TIEMPO_ANIMACION 60.0   /* Duracion maxima en tiempo real de la animacion (s) */
#define MAX_CADENA_ESTADO 32        /* Longitud maxima para textos de estado y modo */

/* =========================================================================
 * 5. TIPOS DE DATOS ESTRUCTURADOS (ENUMS Y STRUCTS)
 * ========================================================================= */

/* Enumeracion para clasificar el resultado de la simulacion */
typedef enum {
    ESTADO_ESTABLE = 1,             /* Orbita circular estable */
    ESTADO_CAIDA,                   /* Velocidad insuficiente: la nave cae */
    ESTADO_ELIPTICA,                /* Velocidad mayor a la ideal: orbita eliptica */
    ESTADO_ESCAPE                   /* Velocidad de escape alcanzada */
} EstadoOrbital;

/* Estructura que almacena el estado fisico y geometrico actual de la simulacion */
typedef struct {
    /* Parametros de entrada (predefinidos o modificados por el usuario) */
    double masa_nave;               /* Mn en kg */
    double altura_orbita;           /* h en m */
    int cantidad_vueltas;           /* Vueltas enteras [1; 10] */
    double aceleracion_lateral;     /* Aceleracion del propulsor auxiliar a (m/s^2) */
    double tiempo_aceleracion;      /* Duracion del pulso t (s) */

    /* Magnitudes gravitatorias y orbitales (MCU) */
    double radio_orbital;           /* Ro = Rmt + h (m) */
    double velocidad_orbital;       /* Velocidad orbital activa v (m/s) */
    double velocidad_ideal;         /* Velocidad ideal dinamica segun altura h (m/s) */
    double velocidad_escape;        /* Velocidad de escape dinamica segun altura h (m/s) */
    double limite_inferior_vel;     /* Videal - 50 m/s (Riesgo de caida) */
    double limite_superior_vel;     /* Videal + 50 m/s (Salida a orbita eliptica) */
    double atraccion_gravitatoria;  /* Fuerza de atraccion gravitacional Fg / Fc (N) */
    double aceleracion_centripeta;  /* Modulo de aceleracion centripeta ac (m/s^2) */
    double periodo_segundos;        /* Periodo orbital T en segundos */
    double periodo_minutos;         /* Periodo orbital T en minutos */

    /* Resultados de la maniobra lateral */
    double velocidad_lateral;       /* Velocidad transversal final (m/s) */
    double desviacion_lateral;      /* Desviacion lateral instantanea (m) */

    /* Variables cinematicas y de posicion para animacion 2D */
    double angulo_theta;            /* Angulo actual theta (rad) */
    double velocidad_angular;       /* Velocidad angular Omega (rad/s) */
    double pos_x_real;              /* Coordenada fisica X en el plano orbital (m) */
    double pos_y_real;              /* Coordenada fisica Y en el plano orbital (m) */
    int pos_x_consola;              /* Columna actual en la matriz 61x25 */
    int pos_y_consola;              /* Fila actual en la matriz 61x25 */

    /* Registro del resultado alcanzado */
    EstadoOrbital resultado_codigo;
    char resultado_texto[MAX_CADENA_ESTADO]; /* "Estable", "Caida", "Eliptica" o "Escape" */
} SistemaOrbital;

/* Estructura para cada registro del historial (hasta 20 simulaciones) */
typedef struct {
    int id_simulacion;              /* Numero correlativo de ejecucion */
    char modo_simulacion[MAX_CADENA_ESTADO]; /* Estandar, Personalizada, Estabilidad, Demo */
    double masa_nave;
    double altura_orbita;
    double velocidad_orbital;
    double periodo_minutos;
    double aceleracion_lateral;
    double tiempo_aceleracion;
    double desviacion_lateral;
    int cantidad_vueltas;
    char resultado[MAX_CADENA_ESTADO];       /* "Estable", "Caida", "Eliptica" o "Escape" */
} RegistroSimulacion;

/* Estructura contenedora del historial con control de tope (20 registros) */
typedef struct {
    RegistroSimulacion registros[MAX_HISTORIAL];
    int cantidad_simulaciones;      /* Contador con limite superior = 20 */
    int total_ejecutadas;           /* Contador historico para asignar ID correlativo */
} HistorialSimulaciones;

/* =========================================================================
 * 6. PROTOTIPOS DE FUNCIONES PUBLICAS DEL MODULO LOGICO (artemis.h)
 * ========================================================================= */

/* Inicializacion y configuracion de parametros */
void inicializar_historial(HistorialSimulaciones *historial);
void cargar_parametros_estandar(SistemaOrbital *sistema);
void configurar_simulacion_personalizada(SistemaOrbital *sistema);
void configurar_prueba_estabilidad(SistemaOrbital *sistema);

/* Motor de calculos fisicos y evaluacion orbital */
void calcular_magnitudes_orbitales(SistemaOrbital *sistema, int usar_velocidad_manual);
void evaluar_estabilidad_orbital(SistemaOrbital *sistema);

/* Ejecucion grafica y submodos de la Opcion 3 */
void ejecutar_animacion_2d(SistemaOrbital *sistema);
void ejecutar_demostraciones_conceptos(SistemaOrbital *sistema, HistorialSimulaciones *historial);

/* Gestion de historial (desplazamiento i = i - 1 al superar 20) e informes */
void guardar_en_historial(HistorialSimulaciones *historial, const SistemaOrbital *sistema, const char *modo);
void mostrar_historial_simulaciones(const HistorialSimulaciones *historial);

/* Pantallas informativas del menu principal */
void mostrar_encabezado_creditos(void);
void mostrar_descripcion_artemis(void);
void mostrar_descripcion_simulador(void);

#endif /* ARTEMIS_H */              /* Cierra el bloque del guard (obligatorio) */
