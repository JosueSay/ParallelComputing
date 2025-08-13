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

// Ajuste de hilos hilo/vehiculos
#define VEHICULOS_POR_HILO 8
#define MIN_HILOS_FASE 1

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

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
 * Retorno: void.
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
 * Retorno: void.
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
 * Retorno: void.
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
 * Detecta choques simples en posiciones NO negativas del mismo carril.
 * Parámetros:
 *   inter: intersección con el estado actual.
 * Retorno:
 *   número de choques detectados (imprime quién chocó).
 *
 * Nota: se ignoran coincidencias en posiciones negativas (cola).
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
 *   t: número de iteración (1..ITERACIONES).
 *   inter: estado de la intersección.
 * Retorno: void.
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
 * Retorno: void.
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
 * Pausa la ejecución por ms milisegundos (opcional y portable).
 * Parámetros:
 *   ms: milisegundos a dormir (0 = no hace nada).
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
 * countActiveVehicles
 * Cuenta vehículos que potencialmente se moverán (semáforo asociado en GREEN).
 * Parámetros:
 *   inter: intersección con el estado actual.
 * Retorno:
 *   número de vehículos con semáforo GREEN.
 */
static int countActiveVehicles(const Interseccion *inter)
{
  int activos = 0;
  for (int i = 0; i < inter->nVehiculos; ++i)
  {
    const Vehiculo *v = &inter->vehiculos[i];
    if (inter->semaforos[v->idSemaforo].estado == GREEN)
    {
      ++activos;
    }
  }
  return activos;
}

/*
 * estimateThreadsByLoad
 * Estima un número de hilos en función de los elementos activos y el máximo disponible.
 * Parámetros:
 *   activos: elementos que se van a procesar (p. ej., vehículos activos).
 *   maxThreads: hilos máximos reportados por OpenMP.
 * Retorno:
 *   número de hilos recomendado para esta fase.
 */
static int estimateThreadsByLoad(int activos, int maxThreads)
{
  if (activos <= 0)
    return MIN_HILOS_FASE;
  int h = activos / VEHICULOS_POR_HILO;
  if (activos % VEHICULOS_POR_HILO)
    ++h;
  if (h < MIN_HILOS_FASE)
    h = MIN_HILOS_FASE;
  if (h > maxThreads)
    h = maxThreads;
  return h;
}

/*
 * main
 * Bucle principal: actualizar semáforos (posible paralelo) -> mover vehículos (paralelo con schedule(runtime))
 * -> log -> choques. Habilita omp_set_dynamic para adaptación.
 * Retorno:
 *   0 si la ejecución finaliza correctamente.
 */
int main(void)
{
  // Ajuste dinámico general (OpenMP decidirá si reduce/aumenta hilos según runtime).
  omp_set_dynamic(1);

  Interseccion inter = crearInterseccion(N_VEHICULOS, N_SEMAFOROS);
  const int maxThreads = omp_get_max_threads();

  for (int t = 1; t <= ITERACIONES; ++t)
  {
    // === Fase A: actualizar semáforos ===
    // Hilos para semáforos: proporcional a la cantidad (normalmente pocos).
    int threadsSem = estimateThreadsByLoad(inter.nSemaforos, maxThreads);
    omp_set_num_threads(threadsSem);
#pragma omp parallel for schedule(static)
    for (int i = 0; i < inter.nSemaforos; ++i)
    {
      actualizarSemaforoTick(&inter.semaforos[i]);
    }

    // === Fase B: mover vehículos (núcleo costoso) ===
    // Estimar cuántos realmente podrían moverse (semáforo asociado en GREEN)
    int activos = countActiveVehicles(&inter);
    int threadsVeh = estimateThreadsByLoad(activos > 0 ? activos : inter.nVehiculos, maxThreads);
    omp_set_num_threads(threadsVeh);

// schedule(runtime): permite elegir dynamic/guided/static vía OMP_SCHEDULE
#pragma omp parallel for schedule(runtime)
    for (int i = 0; i < inter.nVehiculos; ++i)
    {
      moverVehiculoTick(&inter.vehiculos[i], inter.semaforos);
    }

    // === Fase C: logging y detección de choques (secuencial para mantener orden) ===
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
