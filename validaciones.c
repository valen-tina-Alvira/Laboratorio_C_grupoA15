#include <stdio.h>          /* Para printf, scanf y getchar */
#include "validaciones.h"   /* Para que el .c coincida con los prototipos declarados */

void limpiar_buffer(void)   /* Funcion que vacia el buffer de entrada */
{
    int c;                  /* int (no char) porque getchar() devuelve int y puede devolver EOF */
    while ((c = getchar()) != '\n' && c != EOF);  /* Lee y descarta caracteres hasta el Enter o fin de entrada */
}

int leer_entero_rango(int min, int max)   /* Pide un entero y repite hasta que este en [min, max] */
{
    int valor;      /* Aqui scanf guarda el numero ingresado */
    int leidos;     /* Cantidad de valores que scanf logro leer (1 si fue exitoso, 0 si no) */

    while (1) {                              /* Bucle infinito: solo se sale con return */
        leidos = scanf("%d", &valor);        /* Intenta leer un entero y guarda el resultado de la lectura */
        limpiar_buffer();                    /* Descarta el resto de la linea (Enter o texto sobrante) */

        if (leidos != 1) {                   /* Si no se pudo leer un entero (ej: el usuario escribio letras) */
            printf("   [ERROR] Entrada invalida. Ingrese un numero entero [%d-%d]: ", min, max);  /* Avisa del error */
        } else if (valor < min || valor > max) {   /* Si leyo un entero pero esta fuera del rango permitido */
            printf("   [ERROR] Fuera de rango. Ingrese un valor entre %d y %d: ", min, max);      /* Avisa del error */
        } else {                             /* Si el valor es entero y esta dentro del rango */
            return valor;                    /* Devuelve el valor valido y termina la funcion */
        }
    }
}
