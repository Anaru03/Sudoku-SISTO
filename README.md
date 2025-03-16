# Sudoku Validator

## Descripción
Este programa en C implementa un validador de soluciones de Sudoku de 9x9 utilizando multithreading con `pthreads` y OpenMP. Se verifica si la solución ingresada cumple con las reglas del Sudoku revisando filas, columnas y subcuadrantes de 3x3.

## Requisitos
- Sistema operativo Linux o Windows con GCC instalado.
- Conocimientos de programación en C.
- Familiaridad con `pthreads` y OpenMP.

## Compilación
Para compilar el programa, utilice:
```sh
gcc -o SudokuValidator SudokuValidator.c -pthread -fopenmp
```

## Ejecución
Para ejecutarlo con un archivo de entrada:
```sh
./SudokuValidator sudoku.txt
```

## Funcionamiento

### 1. Carga del Sudoku
- Se lee la solución desde un archivo de texto utilizando `open()` y `mmap()`.
- Se almacena en una matriz global de 9x9.

### 2. Verificación de la solución
- Se revisan **filas**, **columnas** y **subcuadrantes de 3x3** para asegurar que contengan los números del 1 al 9.

### 3. Creación de procesos y threads
- Se usa `fork()` para crear un proceso hijo que ejecuta `ps`.
- En el proceso padre, se crea un `pthread` para la validación de columnas.
- Se usa `pthread_join()` para esperar la finalización del `pthread` antes de continuar con la validación de filas.
- Se despliega el ID del thread en ejecución utilizando `syscall(SYS_gettid)`.
- Se crea un segundo `fork()` para ejecutar `ps` antes de finalizar el programa.

### 4. Paralelización con OpenMP
- Se paralelizan los `for` donde sea posible usando `#pragma omp parallel for`.
- Se controla la concurrencia con `omp_set_num_threads()`.
- Se analiza el impacto de `schedule(dynamic)` en la distribución de trabajo.
- Se prueba `omp_set_nested(true)` para evaluar anidamiento de threads.

## Resultados esperados
El programa desplegará:
- La solución del Sudoku en formato de grilla.
- Mensajes indicando el progreso de validación.
- Número de threads y procesos creados.
- Información sobre los `LWP’s` durante la ejecución.
- Un mensaje final indicando si la solución del Sudoku es válida o no.

## Observaciones
- Se debe evitar **race conditions** al compartir recursos entre threads.
- OpenMP maneja un **thread team**, donde el `master thread` coordina la ejecución.
- `schedule(dynamic)` permite una mejor distribución de carga de trabajo.
- `omp_set_nested(true)` puede incrementar la concurrencia pero no siempre mejora el desempeño.

## Autores
- **Ruth de León**
- **Sistemas Operativos - Semestre I, 2025**

