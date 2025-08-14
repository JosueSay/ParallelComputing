
# Resumen de la estrategia

## Idea central

Paralelizamos los **bucles** que procesan colecciones:

- Semáforos: bucle corto, **`schedule(static)`**.
- Vehículos: bucle grande, **`schedule(runtime)`** para poder elegir `dynamic/guided/static` por variable de entorno.

## Dónde está en el código (arch. paralela)

- Activación y adaptación:

  - `omp_set_dynamic(1);` → OpenMP puede ajustar hilos en runtime.
  - `estimateThreadsByLoad(...)` → calculamos **hilos por fase** según carga.
- Paralelismo en semáforos (**Fase A**):

  ```c
  #pragma omp parallel for schedule(static)
  for (int i = 0; i < inter.nSemaforos; ++i) actualizarSemaforoTick(...);
  ```

- Paralelismo en vehículos (**Fase B**, núcleo):

  ```c
  #pragma omp parallel for schedule(runtime)
  for (int i = 0; i < inter.nVehiculos; ++i) moverVehiculoTick(...);
  ```

  Aquí cae todo el beneficio: repartimos las iteraciones del bucle de vehículos entre hilos.
- **Fase C (log y choques)** es **secuencial** para mantener orden y evitar carreras en `printf`.

## Por qué así

- **Encaja con el trabajo dominante**: el bucle de vehículos es grande y sus iteraciones son, en general, independientes.
- **Balance realista**: con `schedule(dynamic|guided)` evitamos hilos ociosos cuando unas iteraciones tardan más que otras.
- **Simplicidad**: no cambiamos reglas del modelo ni agregamos sincronizaciones complejas; solo paralelizamos bucles.

## Qué logramos

- **Aprovechamiento de CPU** y **speedup** mensurable sobre la base secuencial.
- **Flexibilidad**: cambiamos la planificación sin recompilar con `OMP_SCHEDULE` (p.ej. `dynamic,32`).
- **Control**: ajustamos hilos por fase con `estimateThreadsByLoad` y dejamos el log fuera del paralelo.

## Dependencias y sincronización

- Orden de fases: **Semáforos → Vehículos → Log**.
- Las barreras las da el final de cada bucle paralelo; no hay escrituras cruzadas dentro de los bucles (cada hilo toca su índice).

## Riesgos y mitigación

- **Overhead de planificación**: mitigar con `guided` o `dynamic` con *chunks* razonables (`OMP_SCHEDULE`).
- **Desbalance residual**: el **`runtime`** + cálculo de hilos por carga lo reduce.
- **Salidas mezcladas**: evitado al **no imprimir** dentro de regiones paralelas.
- El paralelismo clave está en los **`#pragma omp parallel for`** de las fases A/B; especialmente en el **bucle de vehículos**.
