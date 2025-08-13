# Documentación del Proyecto: Simulación de Tráfico con OpenMP

## 1. Descripción General

Este proyecto consiste en simular el comportamiento de una intersección controlada por **semáforos**, con múltiples **vehículos** que circulan y esperan instrucciones para moverse.
Se utiliza **paralelismo con OpenMP** para manejar en simultáneo el cambio de estados de los semáforos y el movimiento de los vehículos.

La simulación incluye:

- **Paralelismo anidado** para representar múltiples procesos jerárquicos (semáforos y vehículos).
- **Paralelismo dinámico** para ajustar automáticamente el número de hilos según la carga de trabajo.

## 2. Participantes en la simulación

- **Vehículos**: Se mueven según el estado del semáforo que les corresponde.
- **Semáforos**: Controlan el flujo de vehículos, alternando entre los estados **rojo**, **amarillo** y **verde**.
- **Intersección**: Espacio que contiene los semáforos y los vehículos.

## 3. Objetivo del proyecto

El objetivo es:

- Simular correctamente el **ciclo de los semáforos**.
- Mover los vehículos según las reglas del tráfico.
- Ajustar de forma **dinámica** el número de hilos para optimizar recursos.
- Detectar posibles “choques” (conflictos de acceso a memoria compartida o condiciones de carrera).
- Llevar un **registro (log)** de cada iteración mostrando la posición de cada vehículo y el estado de cada semáforo.

## 4. Flujo del proyecto (basado en los pasos de las imágenes)

1. **Definir estructuras de datos**:

   - Estructura para vehículos
   - Estructura para semáforos
   - Estructura para intersección

2. **Inicializar datos**:

   - Crear un conjunto inicial de vehículos.
   - Configurar el estado inicial de cada semáforo.

3. **Comportamiento de semáforos**:

   - Función que cambia el estado periódicamente.
   - Uso de paralelismo para manejar varios semáforos al mismo tiempo.

4. **Comportamiento de vehículos**:

   - Función que actualiza la posición de los vehículos según el estado del semáforo correspondiente.
   - Uso de paralelismo para mover múltiples vehículos simultáneamente.

5. **Simulación del ciclo**:

   - Bucle principal que en cada iteración:

     - Actualiza el estado de los semáforos.
     - Mueve los vehículos.
     - Guarda un log del estado.

6. **Ajuste dinámico de hilos**:

   - Habilitar `omp_set_dynamic(1)`.
   - Ajustar hilos según número de vehículos y cambios en la simulación.

7. **Pruebas y optimización**:

   - Ejecutar con distintas configuraciones de semáforos y vehículos.
   - Medir el rendimiento y optimizar el uso de recursos.

## 5. Qué se debe lograr

- ✅ Sincronizar correctamente semáforos y vehículos para evitar conflictos.
- ✅ Log de cada iteración con:

  - Posición de todos los vehículos.
  - Estado de todos los semáforos.
- ✅ Ajuste eficiente de hilos según la carga.
- ✅ Evitar errores de memoria (condiciones de carrera, accesos no protegidos).
- ✅ Escalabilidad con mayor cantidad de vehículos y semáforos.

## 6. Concepto de “Choques” en la simulación

En este contexto, los “choques” no son colisiones físicas de vehículos, sino **conflictos de concurrencia**:

- **Condiciones de carrera**: cuando varios hilos intentan leer/escribir la misma variable al mismo tiempo sin sincronización.
- **Errores de memoria**: como accesos fuera de rango o uso de punteros inválidos.

Para prevenirlos:

- Usar `#pragma omp critical` o `#pragma omp atomic` cuando se actualicen datos compartidos.
- Usar variables privadas cuando sea necesario (`private`, `firstprivate`).

## 7. Ejemplo de salida esperada (log)

Ejemplo de una iteración:

```bash
Iteración 1:
Vehículo 0 - Posición: 0
Vehículo 1 - Posición: 1
...
Semáforo 0 - Estado: Verde
Semáforo 1 - Estado: Rojo
...
```
