#ifndef VALIDACIONES_H   /* Si VALIDACIONES_H no esta definido todavia, incluye este archivo */
#define VALIDACIONES_H   /* Lo define para que el archivo no se incluya dos veces (guard) */

/* =========================================================================
 * 1. UTILIDADES DE ENTRADA
 * ========================================================================= */
void limpiar_buffer(void);                                  /* Prototipo: descarta lo que quedo en la entrada */
void pausar_enter (void);                                   /* Prototipo: espera que el usuario presione ENTER */
/* =========================================================================
 * 2. LECTURA SEGURA DE DATOS
 * ========================================================================= */
int leer_entero_rango(int min, int max);                    /* Prototipo: pide un entero entre min y max */
double leer_double_rango(double min, double max);           /* Prototipo: pide un real entre min y max */
int leer_si_no(void);                                       /* Prototipo: pide S o N. Devuelve 1 si es "si" y 0 si es "no" */

/* =========================================================================
 * 3. VALIDACIONES COMBINADAS
 * ========================================================================= */
void leer_pulso_lateral(double *aceleracion, double *tiempo,
                        double aceleracion_max, double tiempo_max);  /* Prototipo: pide a y t con la regla combinada */
#endif                 