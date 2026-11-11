#ifndef VALIDACIONES_H   /* Si VALIDACIONES_H no esta definido todavia, incluye este archivo */
    #define VALIDACIONES_H   /* Lo define, para que no se incluya dos veces (guard) */

    void limpiar_buffer(void);                 /* Prototipo: descarta lo que quedo en la entrada */
    int leer_entero_rango(int min, int max);   /* Prototipo: pide un entero entre min y max */

#endif                   /* Cierra el bloque condicional del guard */