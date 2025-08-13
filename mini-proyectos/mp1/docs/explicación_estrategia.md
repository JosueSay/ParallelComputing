# Estrategia elegida: **Paralelismo por datos con bucles paralelos (`parallel for`) + planificación dinámica y/o anidada**

## Problema y modelo de datos

El objetivo es simular una **intersección** donde:

- **Vehículos**: se mueven conforme al estado del semáforo que les corresponde. Cada vehículo tiene identificador, posición y velocidad.
- **Semáforos**: alternan entre estados (verde, amarillo, rojo) con reglas de tiempo predefinidas.
- **Intersección**: contiene la lista de semáforos y vehículos.
- **Log**: registra en cada iteración la posición de los vehículos y el estado de los semáforos.

El ciclo de la simulación:

1. Actualizar estados de semáforos.
2. Mover vehículos según reglas.
3. Registrar el estado y continuar a la siguiente iteración.

## Estrategia de paralelismo por datos

Es un enfoque de paralelismo por datos:

- Las iteraciones de los bucles que procesan colecciones (vehículos y, en menor medida, semáforos) se reparten entre varios hilos.
- La **cláusula `schedule`** controla cómo se distribuyen las iteraciones:
  - `static`: reparto fijo; bajo overhead.
  - `dynamic`: reparto bajo demanda; mejor balance cuando cada iteración cuesta distinto, a cambio de mayor overhead.
  - `guided`: variante de `dynamic` que reduce overhead con *chunks* decrecientes.

Esta estrategia se utiliza por los siguientes incisos:

- **Encaje con el trabajo dominante:** El coste principal está en mover muchos vehículos en cada iteración; sus actualizaciones son, en gran medida, independientes (repartir iteraciones de un bucle entre hilos). ([OpenMP][1], [hpc-wiki.info][5])

- **Balance de carga realista:** El tráfico es irregular (colas, frenadas), por lo que `schedule(dynamic)` o `guided` mejoran el uso de CPU frente a `static`, aunque con un pequeño overhead conocido y aceptable para cargas variables. ([Ian Finlayson][2], [inf.ufsc.br][6])

- **Simplicidad y escalabilidad:** Empezamos paralelizando el bucle de vehículos (simple, clara ganancia) y, si hace falta, añadimos un segundo nivel anidado por semáforo. El enfoque respeta el flujo del curso sobre dinámico y anidado, sin introducir complejidad innecesaria. ([OpenMP][3])

El núcleo de la simulación es el **bucle que procesa todos los vehículos en cada iteración**, el cual se **paraleliza** con `#pragma omp parallel for` para distribuir las iteraciones entre hilos. Se emplea **planificación dinámica** (`schedule(dynamic)` o `schedule(guided)`) para balancear la carga cuando el tiempo de mover cada vehículo es variable (por ejemplo, vehículos detenidos o en colas).

Opcionalmente, puede paralelizarse también el **bucle de semáforos** (normalmente menos costoso) o habilitar **paralelismo anidado** para casos de fuerte asimetría por semáforo (unos con muchas colas y otros vacíos):

- **Nivel 1**: procesar semáforos o carriles en paralelo.
- **Nivel 2**: procesar los vehículos de cada semáforo en paralelo.

En todos los casos, la **coordinación entre fases** —actualizar semáforos -> mover vehículos -> logging— se mantiene clara, con sincronización donde exista dependencia.

## Dependencias entre fases y puntos de sincronización

- **Semáforos -> Vehículos**: el movimiento de vehículos depende del estado actualizado de su semáforo, por lo que esta fase debe completarse antes de iniciar la fase de movimiento.
- **Vehículos -> Log**: el registro de estado depende de los datos actualizados de vehículos y semáforos; se ejecuta después de ambas fases.
- Los puntos de sincronización se limitan al final de cada fase para evitar condiciones de carrera.

## Riesgos y mitigaciones

### a. Condiciones de carrera

- **Riesgo**: múltiples hilos escribiendo sobre datos compartidos (logs, contadores globales).
- **Mitigación**: usar variables privadas, `reduction` para acumuladores, y buffers por hilo para consolidar en una sola escritura al final.

### b. Overhead de planificación dinámica

- **Riesgo**: `dynamic` y `guided` añaden coste de reparto.
- **Mitigación**: ajustar tamaño de *chunk* para reducir overhead, probar `static` si la carga es uniforme.

### c. Desbalance residual

- **Riesgo**: semáforos con colas muy desiguales.
- **Mitigación**: activar anidado para procesar vehículos por semáforo en paralelo o aplicar partición equilibrada de datos.

### d. Sobrecarga por anidado

- **Riesgo**: demasiados hilos activos en paralelo.
- **Mitigación**: limitar hilos por nivel con `omp_set_num_threads` y medir speedup efectivo antes de adoptarlo.

## Comparativa con `sections` y `tasks`

- **`sections`**: útil cuando hay pocas tareas heterogéneas (ej. semáforos y vehículos como bloques distintos). No escala bien cuando una de las tareas domina el tiempo, como es el caso aquí, porque el paralelismo máximo queda limitado al número de secciones.
- **`tasks`**: permite generar trabajos en tiempo de ejecución; es más flexible, pero con más sobrecarga y complejidad de implementación. Ideal para cargas altamente irregulares con número de tareas variable.
- **Paralelismo por datos**: encaja mejor aquí porque el trabajo principal es iterativo sobre una colección grande (vehículos) y puede repartirse de forma natural y escalable con `parallel for` y planificación dinámica.

## Plan de experimentación

1. **Variar número de hilos**: probar desde 1 hasta el máximo disponible (`OMP_NUM_THREADS`).
2. **Probar políticas de planificación**:
   - `static`, `dynamic`, `guided`.
   - Distintos tamaños de *chunk*.

3. **Medir tiempo por fase**:
   - Actualización de semáforos.
   - Movimiento de vehículos.
   - Logging.

4. **Evaluar anidado**:
   - Habilitar/deshabilitar `omp_set_nested(1)`.
   - Limitar hilos por nivel.

5. **Métricas a registrar**:

   - Tiempo total.
   - Speedup respecto a la versión secuencial.
   - Escalabilidad con distintos tamaños de entrada (vehículos y semáforos).

## Conclusión

El **paralelismo por datos con `parallel for` y planificación dinámica** es la estrategia más adecuada para este proyecto porque:

- Se alinea con el patrón de trabajo dominante (procesar una gran cantidad de vehículos en cada iteración).
- Aprovecha mejor el hardware disponible al equilibrar la carga entre hilos.
- Permite incorporar paralelismo anidado de forma controlada para escenarios con alta asimetría.
- Mantiene un código claro y fácil de justificar en un entorno docente, maximizando rendimiento con mínima complejidad.

[1]: https://www.openmp.org/spec-html/5.0/openmpsu41.html "2.9.2 Worksharing-Loop Construct"
[2]: https://ianfinlayson.net/class/cpsc425/notes/12-scheduling "OpenMP Scheduling"
[3]: https://www.openmp.org/wp-content/uploads/OpenMP-API-Specification-5.0.pdf "OpenMP Application Programming Interface"
[5]: https://hpc-wiki.info/hpc/OpenMP "OpenMP"
[6]: https://www.inf.ufsc.br/~bosco/ensino/ine5645/OpenMP_Dynamic_Scheduling.pdf "OpenMP Scheduling"
