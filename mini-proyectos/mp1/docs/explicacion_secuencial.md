# Archivo `simulacion_secuencial.c`

## Constantes y configuración

```c
#define N_SEMAFOROS 4
#define N_VEHICULOS 20
#define ITERACIONES 10
#define SLEEP_MS 0
```

- **`N_SEMAFOROS`** -> Número total de semáforos en la simulación (4).
- **`N_VEHICULOS`** -> Número total de vehículos en la simulación (20).
- **`ITERACIONES`** -> Cantidad de ciclos completos que se simularán (10).
- **`SLEEP_MS`** -> Tiempo de espera entre iteraciones en milisegundos (0 significa sin pausa).

```c
#define DUR_RED 2
#define DUR_GREEN 2
#define DUR_YELLOW 1
```

- Son duraciones en las iteraciones de la simulación que un semáforo mantiene un color antes de cambiar:
  - **`DUR_RED` = 2** -> Luz roja dura 2 iteraciones.
  - **`DUR_GREEN` = 2** -> Luz verde dura 2.
  - **`DUR_YELLOW` = 1** -> Luz amarilla dura 1.

```c
#define PATRON_INICIAL(i) ((i) % 4 == 0 ? 1 : (i) % 4 == 1 ? 1\
                                            : (i) % 4 == 2   ? 0\
                                            : 2)
```

- Devuelve el **estado inicial** de un semáforo según su índice `i`.
- Los resultados para el ejemplo:

  - Si `(i) % 4 == 0` -> Estado `1` (verde).
  - Si `(i) % 4 == 1` -> Estado `1` (verde).
  - Si `(i) % 4 == 2` -> Estado `0` (rojo).
  - Si `(i) % 4 == 3` -> Estado `2` (amarillo).

## Librerías

```c
#define _POSIX_C_SOURCE 199309L
#include <time.h>
```

- Básicamente, le dice al compilador: *“quiero que habilites funciones POSIX de esta versión o anterior”* para asegurarnois de usar funciones de `time.h` que es una biblioteca para funcioens de tiempo.

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
```

- **`stdio.h`** -> Entrada/Salida estándar.
- **`stdlib.h`** -> Funciones de utilidades generales.
- **`stdbool.h`** -> Soporte para tipo booleano.

```c
#include <omp.h>
```

- Esto incluye la biblioteca de OpenMP (Open Multi-Processing), que permite programar paralelismo.

## Estructuras

```c
typedef enum
{
  RED = 0,
  GREEN = 1,
  YELLOW = 2
} EstadoSemaforo;
```

- **`enum`** -> Enumeración: define un conjunto de valores con nombre mediante enteros.

```c
typedef struct
{
  int id;
  EstadoSemaforo estado;
  int tiempoRestante;
  int durRed, durGreen, durYellow;
} Semaforo;
```

- **`Semaforo`** es una estructura que describe un semáforo individual.
- Campos:
  - `id` -> Identificador único del semáforo.
  - `estado` -> Color actual (usa `EstadoSemaforo`).
  - `tiempoRestante` -> Cuántas iteraciones le quedan en el estado actual antes de cambiar.
  - `durRed`, `durGreen`, `durYellow` -> Duración personalizada para cada color en ese semáforo.

Esto permite que cada semáforo tenga tiempos diferentes, no todos son iguales.

```c
typedef struct
{
  int id;
  int posicion;
  int idSemaforo;
} Vehiculo;
```

- **`Vehiculo`** es la estructura que describe un coche o unidad en la simulación.
- Campos:
  - `id` -> Identificador único del vehículo.
  - `posicion` -> Posición del vehículo en la vía (puede ser en metros, celdas, etc. dependiendo del modelo).
  - `idSemaforo` -> Identificador del semáforo al que está asociado o frente al que está.

```c
typedef struct
{
  int nSemaforos, nVehiculos;
  Semaforo *semaforos;
  Vehiculo *vehiculos;
} Interseccion;
```

- **`Interseccion`** representa el estado completo en una iteración de la simulación. Acá unimos todo, la lista de semáforos y lista de vehículos simulados en el momento.
- Campos:
  - `nSemaforos` -> Número de semáforos.
  - `nVehiculos` -> Número de vehículos.
  - `semaforos` -> Puntero a un arreglo dinámico de `Semaforo`.
  - `vehiculos` -> Puntero a un arreglo dinámico de `Vehiculo`.
