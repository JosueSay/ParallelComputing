# Instalación y uso de GCC en WSL

Este documento describe los pasos necesarios para instalar el compilador GCC en WSL (Windows Subsystem for Linux) y compilar y ejecutar un programa en C.

## 1. Actualizar los repositorios

Antes de instalar GCC, se recomienda actualizar la lista de paquetes disponibles:

```bash
sudo apt update
```

Este comando sincroniza los repositorios de paquetes para asegurar que se instalen las versiones más recientes.

## 2. Instalar build-essential

El paquete `build-essential` incluye GCC, G++ y otras herramientas necesarias para compilar programas en C y C++:

```bash
sudo apt install build-essential
```

Durante la instalación, el sistema puede solicitar confirmación. Escriba `Y` y presione Enter para continuar.

## 3. Compilar un programa en C

Una vez instalado GCC, se puede compilar un archivo en C. Supongamos que el archivo se llama `programa.c`:

```bash
gcc programa.c -o programa
```

Este comando crea un archivo ejecutable llamado `programa` en el mismo directorio.

## 4. Ejecutar el programa

Para ejecutar el programa compilado:

```bash
./programa
```

El prefijo `./` indica que el ejecutable se encuentra en el directorio actual.

## 5. Verificar la instalación

Para confirmar que GCC se instaló correctamente, puede ejecutar:

```bash
gcc --version
```

Este comando mostrará la versión de GCC instalada en el sistema.
