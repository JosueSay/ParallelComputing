// Constantes y configuración
#define N_SEMAFOROS 4
#define N_VEHICULOS 20
#define ITERACIONES 10
#define SLEEP_MS 0

// Configuración de semáforo
#define DUR_RED 2
#define DUR_GREEN 2
#define DUR_YELLOW 1
#define PATRON_INICIAL(i) ((i) % 4 == 0 ? 1 : (i) % 4 == 1 ? 1 \
                                          : (i) % 4 == 2   ? 0 \
                                                           : 2)
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Estados de semáforo
typedef enum
{
  RED = 0,
  GREEN = 1,
  YELLOW = 2
} EstadoSemaforo;

// Estructura de un semáforo
typedef struct
{
  int id;
  EstadoSemaforo estado;
  int tiempoRestante; // iteraciones restantes (del actual)
  int durRed, durGreen, durYellow;
} Semaforo;

// Estructura de un vehículo
typedef struct
{
  int id;
  int posicion;
  int idSemaforo;
} Vehiculo;

// Estructura de una iteración (semáforos + vehículos)
typedef struct
{
  int nSemaforos, nVehiculos;
  Semaforo *semaforos;
  Vehiculo *vehiculos;
} Interseccion;

/*
 * setEstadoInicialSemaforo
 * Establece el estado inicial del semáforo y su contador de tiempo.
 * Parámetros:
 *   s: puntero al semáforo a inicializar.
 *   e: estado inicial (RED/GREEN/YELLOW).
 */
static void setEstadoInicialSemaforo(Semaforo *s, EstadoSemaforo e)
{
  s->estado = e;
  s->tiempoRestante = (e == RED)     ? s->durRed
                      : (e == GREEN) ? s->durGreen
                                     : s->durYellow;
}

/*
 * actualizarSemaforoTick
 * Avanza un tick el semáforo; si expira su tiempo, cambia al siguiente estado.
 * Parámetros:
 *   s: puntero al semáforo a actualizar.
 */
static void actualizarSemaforoTick(Semaforo *s)
{
  if (--(s->tiempoRestante) > 0)
    return;
  if (s->estado == RED)
  {
    s->estado = GREEN;
    s->tiempoRestante = s->durGreen;
  }
  else if (s->estado == GREEN)
  {
    s->estado = YELLOW;
    s->tiempoRestante = s->durYellow;
  }
  else
  {
    s->estado = RED;
    s->tiempoRestante = s->durRed;
  }
}

/*
 * moverVehiculoTick
 * Mueve un vehículo un paso si el semáforo asociado está en GREEN.
 * Parámetros:
 *   v: puntero al vehículo a mover.
 *   semaforos: arreglo de semáforos (solo lectura).
 */
static void moverVehiculoTick(Vehiculo *v, const Semaforo *semaforos)
{
  const Semaforo *s = &semaforos[v->idSemaforo];
  if (s->estado == GREEN)
  {
    v->posicion += 1;
  }
}

/*
 * detectarChoques
 * Parámetros:
 *   inter: intersección con el estado actual.
 * Retorno:
 *   número de choques detectados
 */
static int detectarChoques(const Interseccion *inter)
{
  int choques = 0;
  for (int i = 0; i < inter->nVehiculos; ++i)
  {
    const Vehiculo *a = &inter->vehiculos[i];
    for (int j = i + 1; j < inter->nVehiculos; ++j)
    {
      const Vehiculo *b = &inter->vehiculos[j];
      if (a->idSemaforo == b->idSemaforo &&
          a->posicion == b->posicion &&
          a->posicion >= 0)
      {
        ++choques;
        printf("  [ALERTA] Choque V%d y V%d en S%d (pos=%d)\n",
               a->id, b->id, a->idSemaforo, a->posicion);
      }
    }
  }
  return choques;
}

/*
 * imprimirEstado
 * Imprime el log de la iteración en el formato solicitado.
 * Parámetros:
 *   t: número de iteración.
 *   inter: estado de la intersección.
 */
static void imprimirEstado(int t, const Interseccion *inter)
{
  printf("iter%d:\n", t);
  for (int i = 0; i < inter->nVehiculos; ++i)
  {
    const Vehiculo *v = &inter->vehiculos[i];
    printf("Vehículo %d - Posición: %d\n", v->id, v->posicion);
  }
  for (int s = 0; s < inter->nSemaforos; ++s)
  {
    printf("Semáforo %d - Estado: %d\n", s, inter->semaforos[s].estado);
  }
  puts("");
}

/*
 * crearInterseccion
 * Crea e inicializa la intersección con semáforos y vehículos.
 * Parámetros:
 *   nVehiculos: cantidad de vehículos.
 *   nSemaforos: cantidad de semáforos.
 * Retorno:
 *   Interseccion inicializada (con memoria reservada).
 *
 * Inicialización:
 * - Semáforos con duraciones fijas y estado inicial por patrón.
 * - Vehículos en colas escalonadas por carril: 0, -1, -2, ...
 */
static Interseccion crearInterseccion(int nVehiculos, int nSemaforos)
{
  Interseccion inter;
  inter.nSemaforos = nSemaforos;
  inter.nVehiculos = nVehiculos;
  inter.semaforos = (Semaforo *)malloc(sizeof(Semaforo) * nSemaforos);
  inter.vehiculos = (Vehiculo *)malloc(sizeof(Vehiculo) * nVehiculos);

  for (int i = 0; i < nSemaforos; ++i)
  {
    Semaforo *s = &inter.semaforos[i];
    s->id = i;
    s->durRed = DUR_RED;
    s->durGreen = DUR_GREEN;
    s->durYellow = DUR_YELLOW;
    setEstadoInicialSemaforo(s, (EstadoSemaforo)PATRON_INICIAL(i));
  }

  for (int i = 0; i < nVehiculos; ++i)
  {
    Vehiculo *v = &inter.vehiculos[i];
    v->id = i;
    v->idSemaforo = i % nSemaforos;
    int indiceEnCola = i / nSemaforos; // 0,1,2,... por carril
    v->posicion = -indiceEnCola;       // evita choques al inicio
  }

  return inter;
}

/*
 * destruirInterseccion
 * Libera la memoria asociada a la intersección.
 * Parámetros:
 *   inter: puntero a la intersección a destruir.
 */
static void destruirInterseccion(Interseccion *inter)
{
  free(inter->semaforos);
  free(inter->vehiculos);
  inter->semaforos = NULL;
  inter->vehiculos = NULL;
}

/*
 * sleepMs
 * Pausa la ejecución por ms
 * Parámetros:
 *   ms: milisegundos
 * Retorno: void.
 */
static void sleepMs(int ms)
{
  if (ms <= 0)
    return;
#if defined(_WIN32) || defined(_WIN64)
  Sleep(ms);
#else
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
#endif
}

/*
 * main
 * Bucle principal de simulación: actualizar semáforos -> mover vehículos -> log -> choques.
 * Retorno:
 *   0 si la ejecución finaliza correctamente.
 */
int main(void)
{
  Interseccion inter = crearInterseccion(N_VEHICULOS, N_SEMAFOROS);

  for (int t = 1; t <= ITERACIONES; ++t)
  {
    // Paso 1: actualizar semáforos
    for (int i = 0; i < inter.nSemaforos; ++i)
    {
      actualizarSemaforoTick(&inter.semaforos[i]);
    }

    // Paso 2: mover vehículos según su semáforo
    for (int i = 0; i < inter.nVehiculos; ++i)
    {
      moverVehiculoTick(&inter.vehiculos[i], inter.semaforos);
    }

    // Paso 3: logging y chequeo de choques
    imprimirEstado(t, &inter);
    int choques = detectarChoques(&inter);
    if (choques == 0)
    {
      puts("Sin choques en esta iteración.\n");
    }

    sleepMs(SLEEP_MS);
  }

  destruirInterseccion(&inter);
  return 0;
}
