#include <stdio.h>          /* Para printf, fgets y getchar */
#include <stdlib.h>         /* Para strtol, strtod, exit y EXIT_FAILURE */
#include <string.h>         /* Para strlen y strchr */
#include <errno.h>          /* Para errno y ERANGE (detectar numeros demasiado grandes) */
#include <ctype.h>          /* Para isspace (saltar espacios al final) */
#include "validaciones.h"   /* Para que este .c coincida con los prototipos declarados */

/* =========================================================================
 * 1. CONSTANTES LOCALES
 * ========================================================================= */
#define TAM_LINEA 64        /* Tamano maximo del texto que se lee por teclado */

/* =========================================================================
 * 2. FUNCIONES AUXILIARES INTERNAS
 * ========================================================================= */
void limpiar_buffer(void)  
{
    int c;                  
    while ((c = getchar()) != '\n' && c != EOF);  /* Descarta caracteres hasta el Enter o el fin de entrada */
}

/* Lee una linea de texto. Devuelve 1 si entro completa y 0 si era demasiado larga */
static int leer_linea(char *destino, int tam)   /* static: solo se usa dentro de este archivo */
{
    size_t largo;           

    if (fgets(destino, tam, stdin) == NULL) {   /* fgets devuelve NULL si se cerro la entrada (EOF) */
        printf("\n   [ERROR] Se cerro la entrada de datos. Fin del programa.\n");  /* Avisa el motivo */
        exit(EXIT_FAILURE); /* Termina el programa: sin entrada no se puede seguir pidiendo datos */
    }

    largo = strlen(destino);                    /* Calcula cuantos caracteres se leyeron */
    if (largo > 0 && destino[largo - 1] == '\n') {   /* Si la linea termina en Enter, entro completa */
        destino[largo - 1] = '\0';              /* Reemplaza el Enter por el fin de cadena */
        return 1;                               /* Linea valida en cuanto a longitud */
    }

    limpiar_buffer();       /* La linea era mas larga que el arreglo: descarta lo que sobro */
    return 0;               /* Avisa que la linea fue demasiado larga */
}

/* =========================================================================
 * 3. LECTURA SEGURA DE ENTEROS Y REALES
 * ========================================================================= */

int leer_entero_rango(int min, int max)   /* Repite hasta obtener un entero dentro de [min, max] */
{
    char linea[TAM_LINEA];  /* Aqui se guarda el texto ingresado */
    char *fin;              /* strtol deja aqui el puntero al primer caracter que no pudo convertir */
    long valor;             /* Numero convertido (long para detectar desbordes antes de achicarlo a int) */

    while (1) {             /* Bucle: solo se sale con return cuando el dato es valido */
        if (!leer_linea(linea, TAM_LINEA)) {    /* Si el texto fue demasiado largo */
            printf("   [ERROR] Entrada demasiado larga. Ingrese un numero entero [%d-%d]: ", min, max);
            continue;       /* Vuelve a pedir el dato */
        }

        errno = 0;                          /* Borra errores previos para detectar los de strtol */
        valor = strtol(linea, &fin, 10);    /* Convierte el texto a entero en base 10 */

        while (isspace((unsigned char)*fin)) {    /* Permite espacios al final ("5 ") */
            fin++;                          
        }

        if (fin == linea || *fin != '\0') { /* Si no convirtio nada, o sobraron caracteres (ej: "12abc") */
            printf("   [ERROR] Entrada invalida. Ingrese un numero entero [%d-%d]: ", min, max);
        } else if (errno == ERANGE || valor < min || valor > max) {  /* Si es enorme o esta fuera del rango */
            printf("   [ERROR] Fuera de rango. Ingrese un valor entre %d y %d: ", min, max);
        } else {            /* Si es un entero completo y dentro del rango */
            return (int)valor;              /* Devuelve el valor ya validado */
        }
    }
}

double leer_double_rango(double min, double max)   /* Repite hasta obtener un real dentro de [min, max] */
{
    char linea[TAM_LINEA];  /* Aqui se guarda el texto ingresado */
    char *fin;              /* strtod deja aqui el puntero al primer caracter que no pudo convertir */
    double valor;           /* Numero real convertido */

    while (1) {             /* Bucle: solo se sale con return cuando el dato es valido */
        if (!leer_linea(linea, TAM_LINEA)) {    /* Si el texto fue demasiado largo */
            printf("   [ERROR] Entrada demasiado larga. Ingrese un numero [%.2f - %.2f]: ", min, max);
            continue;       /* Vuelve a pedir el dato */
        }

        if (strchr(linea, ',') != NULL) {   /* Si el usuario escribio coma decimal */
            printf("   [ERROR] Use punto como separador decimal (ej: 3.5). Reingrese [%.2f - %.2f]: ", min, max);
            continue;       /* Vuelve a pedir el dato */
        }

        valor = strtod(linea, &fin);        /* Convierte el texto a numero real */

        while (isspace((unsigned char)*fin)) {  /* Mientras queden espacios al final del texto */
            fin++;                          /* Los salta */
        }

        if (strspn(linea, "0123456789+-. ") != strlen(linea)) {  /* Si hay algun caracter que no sea digito, signo, punto o espacio */
            printf("   [ERROR] Entrada invalida. Use solo digitos y punto [%.2f - %.2f]: ", min, max);  /* Avisa el error */
            continue;       /* Vuelve a pedir el dato */
        }
        
        if (fin == linea || *fin != '\0') { /* Si no convirtio nada, o sobraron caracteres (ej: "3.5x") */
            printf("   [ERROR] Entrada invalida. Ingrese un numero [%.2f - %.2f]: ", min, max);
        } else if (!(valor >= min && valor <= max)) {  /* Fuera de rango; tambien atrapa "nan", que falla toda comparacion */
            printf("   [ERROR] Fuera de rango. Ingrese un valor entre %.2f y %.2f: ", min, max);
        } else {            /* Si es un real completo y dentro del rango */
            return valor;                   /* Devuelve el valor ya validado */
        }
    }
}



/* =========================================================================
 * 4. CONFIRMACIONES Y PAUSAS
 * ========================================================================= */
int leer_si_no(void)        /* Repite hasta que el usuario responda S o N */
{
    char linea[TAM_LINEA];  /* Aqui se guarda el texto ingresado */

    while (1) {             /* Bucle: solo se sale con return */
        if (!leer_linea(linea, TAM_LINEA)) {    /* Si el texto fue demasiado largo */
            printf("   [ERROR] Entrada demasiado larga. Responda S o N: ");  /* Avisa el error */
            continue;       /* Vuelve a pedir el dato */
        }

        if (strlen(linea) == 1) {               /* Solo se acepta un unico caracter */
            if (linea[0] == 's' || linea[0] == 'S') {   /* Si respondio S o s */
                return 1;   /* Devuelve 1 (verdadero: "si") */
            }
            if (linea[0] == 'n' || linea[0] == 'N') {   /* Si respondio N o n */
                return 0;   /* Devuelve 0 (falso: "no") */
            }
        }

        printf("   [ERROR] Respuesta invalida. Responda S o N: ");  /* Cualquier otra cosa es invalida */
    }
}

void pausar_enter (void)    /* Pausa hasta que el usuario presione ENTER */
{
    char linea[TAM_LINEA];  /* Aqui se guarda (y se descarta) lo que escriba el usuario */

    leer_linea(linea, TAM_LINEA);   /* Lee la linea completa, asi no queda texto sobrante en el buffer */
}

/* =========================================================================
 * 5. VALIDACION COMBINADA: PULSO LATERAL
 * ========================================================================= */
/* Pide aceleracion a y duracion t. Regla: si a = 0, t se asigna a 0; si a > 0, t debe ser > 0 */
void leer_pulso_lateral(double *aceleracion, double *tiempo,
                        double aceleracion_max, double tiempo_max)
{
    printf("   Aceleracion lateral a [0 - %.2f] m/s^2: ", aceleracion_max);  /* Pide la aceleracion */
    *aceleracion = leer_double_rango(0.0, aceleracion_max);   /* Guarda el valor validado en la variable del que llama */

    if (*aceleracion == 0.0) {      /* Si no hay propulsion lateral */
        *tiempo = 0.0;              /* El tiempo se asigna a 0 automaticamente */
        printf("   Aceleracion nula: la duracion del pulso se asigna a 0 s.\n");  /* Informa al usuario */
        return;                     /* No hace falta pedir la duracion */
    }

    do {                            /* Si a > 0, repite hasta que t sea mayor que 0 */
        printf("   Duracion del pulso t (0 - %.2f] s: ", tiempo_max);   /* Pide la duracion */
        *tiempo = leer_double_rango(0.0, tiempo_max);   /* Valida el rango [0, tiempo_max] */
        if (*tiempo <= 0.0) {       /* t = 0 no es valido cuando hay aceleracion */
            printf("   [ERROR] Con aceleracion mayor que 0, la duracion debe ser mayor que 0.\n");
        }
    } while (*tiempo <= 0.0);       /* Vuelve a pedir mientras t no sea positivo */
}