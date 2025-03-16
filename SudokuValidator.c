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

}
