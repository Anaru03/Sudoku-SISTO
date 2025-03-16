#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <omp.h>

#define SIZE 9

// Matriz para almacenar el Sudoku
int sudoku[SIZE][SIZE];

// Estructura para pasar argumentos a los threads
typedef struct {
    int index;
} thread_arg_t;

void* check_rows(void* arg);
void* check_columns(void* arg);
void* check_subgrids(void* arg);
void read_sudoku_from_file(const char* filename);
void print_sudoku();
int validate_array(int* arr);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_sudoku>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Leer el Sudoku desde el archivo
    read_sudoku_from_file(argv[1]);
    print_sudoku();

    // Crear hilos para verificar filas y columnas
    pthread_t tid_columns, tid_rows;
    pthread_create(&tid_columns, NULL, check_columns, NULL);
    pthread_create(&tid_rows, NULL, check_rows, NULL);
    
    // Esperar a que los hilos terminen
    pthread_join(tid_columns, NULL);
    pthread_join(tid_rows, NULL);
    
    // Validar los subgrids en el hilo principal
    check_subgrids(NULL);
    
    printf("Se ha realizado la validación de Sudoku completada.\n");
    return 0;
}
