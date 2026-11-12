#ifndef VALIDACIONES_H   /* Si VALIDACIONES_H no esta definido todavia, incluye este archivo */
#define VALIDACIONES_H   /* Lo define para que el archivo no se incluya dos veces (guard) */

void limpiar_buffer(void);                                  /* Prototipo: descarta lo que quedo en la entrada */
int leer_entero_rango(int min, int max);                    /* Prototipo: pide un entero entre min y max */
double leer_double_rango(double min, double max);           /* Prototipo: pide un real entre min y max */
void leer_pulso_lateral(double *aceleracion, double *tiempo,
                        double aceleracion_max, double tiempo_max);  /* Prototipo: pide a y t con la regla combinada */

#endif                   /* Cierra el bloque abierto por #ifndef */