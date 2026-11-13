# Makefile del Simulador Artemis (Laboratorio_C_grupoA15)
# La animacion se muestra en una VENTANA GRAFICA con la libreria raylib
# (incluida dentro de la carpeta raylib/ del proyecto, compilada en forma
# estatica: el ejecutable no necesita DLL ni descargar nada).
#
# Compilar:            mingw32-make
# Ejecutar:            simulador_artemis.exe

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -DUSAR_RAYLIB -Iraylib/include
TARGET  = simulador_artemis.exe
SRC     = main.c artemis.c validaciones.c simulador.c
HDR     = artemis.h validaciones.h
LIBS    = -Lraylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean:
	del /Q $(TARGET)
