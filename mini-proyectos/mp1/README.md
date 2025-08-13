# Simulación de Semáforos y Vehículos 🚦🚗

Este proyecto simula el comportamiento de semáforos y vehículos en una intersección. Incluye versiones **secuencial** y **paralela** usando OpenMP.

## 📁 Estructura del proyecto

```bash
.
├── README.md
├── docs
│   ├── explicacion_secuencial.md
│   ├── explicación_estrategia.md
│   └── guia.md
├── log_paralela.log
├── log_secuential.log
├── simulacion_paralela.c
├── simulacion_paralela.o
├── simulacion_secuencial.c
└── simulacion_secuencial.o
```

* `docs/` -> Documentación detallada del proyecto.
* `simulacion_secuencial.c` -> Código de la simulación secuencial.
* `simulacion_paralela.c` -> Código de la simulación paralela (OpenMP).
* `*.o` -> Archivos compilados.
* `log_*.log` -> Salidas de ejecución de cada simulación.

## 💻 Compilar y ejecutar

### Secuencial

Compilar:

```bash
gcc -O2 -std=c11 -Wall -Wextra -o simulacion_secuencial.o simulacion_secuencial.c
```

Ejecutar:

```bash
./simulacion_secuencial.o > log_secuential.log
```

### Paralelo (OpenMP)

Compilar:

```bash
gcc -O2 -std=c11 -fopenmp -o simulacion_paralela.o simulacion_paralela.c
```

Ejecutar:

```bash
./simulacion_paralela.o > log_paralela.log
```

## 📝 Logs de ejecución

* `log_secuential.log` -> Salida de la versión secuencial.
* `log_paralela.log` -> Salida de la versión paralela.

## ⚙️ Requisitos

* GCC 11 o superior (compatible con C11)
* OpenMP (para la versión paralela)
* Sistema operativo: Linux, macOS o Windows con WSL

## 📖 Documentación

Para más detalles sobre la implementación y la estrategia utilizada, revisa los archivos dentro de `docs/`:

* `explicacion_secuencial.md` -> Detalle de la simulación secuencial.
* `explicación_estrategia.md` -> Estrategia de manejo de semáforos y vehículos.
* `guia.md` -> Guía de uso y ejemplos.
