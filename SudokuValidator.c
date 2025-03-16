#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <omp.h>
#include <sys/wait.h>

#define SIZE 9

// Matriz para almacenar el Sudoku
int sudoku[SIZE][SIZE];
int valid = 1; // Variable global para verificar si el Sudoku es válido

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
void execute_ps_command(pid_t parent_pid);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <archivo_sudoku>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Leer el Sudoku desde el archivo
    read_sudoku_from_file(argv[1]);
    print_sudoku();

    // Obtener el PID del proceso actual
    pid_t parent_pid = getpid();
    
    // Crear un proceso hijo para ejecutar el comando ps
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        execute_ps_command(parent_pid);
        exit(EXIT_SUCCESS);
    }

    // Crear hilos para verificar filas y columnas
    pthread_t tid_columns, tid_rows;
    pthread_create(&tid_columns, NULL, check_columns, NULL);
    pthread_create(&tid_rows, NULL, check_rows, NULL);
    
    // Esperar a que los hilos terminen
    pthread_join(tid_columns, NULL);
    pthread_join(tid_rows, NULL);
    
    // Validar los subgrids en el hilo principal
    check_subgrids(NULL);
    
    // Esperar al proceso hijo
    wait(NULL);
    
    if (valid) {
        printf("\n✅ La solución del Sudoku es VÁLIDA.\n");
    } else {
        printf("\n❌ La solución del Sudoku es INVÁLIDA.\n");
    }
    
    return 0;
}

// Función para ejecutar el comando ps
void execute_ps_command(pid_t parent_pid) {
    char parent_pid_str[10];
    sprintf(parent_pid_str, "%d", parent_pid);
    execlp("ps", "ps", "-p", parent_pid_str, "-lLf", NULL);
    perror("Error ejecutando ps");
    exit(EXIT_FAILURE);
}

// Función para leer el Sudoku desde un archivo usando mmap()
void read_sudoku_from_file(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    char* data = mmap(NULL, SIZE * SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error en mmap");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            sudoku[i][j] = data[i * SIZE + j] - '0';
        }
    }
    
    munmap(data, SIZE * SIZE);
    close(fd);
}

// Función auxiliar para imprimir el Sudoku con formato mejorado
void print_sudoku() {
    printf("Sudoku a validar:\n");
    printf("+-------+-------+-------+\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (j % 3 == 0) printf("| ");
            printf("%d ", sudoku[i][j]);
        }
        printf("|\n");
        if ((i + 1) % 3 == 0) {
            printf("+-------+-------+-------+\n");
        }
    }
}

// Función auxiliar para validar si un arreglo contiene los números 1-9 sin repetir
int validate_array(int* arr) {
    int seen[SIZE] = {0};
    for (int i = 0; i < SIZE; i++) {
        if (arr[i] < 1 || arr[i] > 9 || seen[arr[i] - 1]) {
            return 0;
        }
        seen[arr[i] - 1] = 1;
    }
    return 1;
}

// Función para revisar las filas usando OpenMP
void* check_rows(void* arg) {
    printf("Revisando filas...\n");
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        int row[SIZE];
        for (int j = 0; j < SIZE; j++) {
            row[j] = sudoku[i][j];
        }
        if (!validate_array(row)) {
            printf("❌ Fila %d inválida.\n", i + 1);
            valid = 0;
        }
    }
    return NULL;
}

// Función para revisar las columnas usando OpenMP
void* check_columns(void* arg) {
    printf("Revisando columnas...\n");
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        int column[SIZE];
        for (int j = 0; j < SIZE; j++) {
            column[j] = sudoku[j][i];
        }
        if (!validate_array(column)) {
            printf("❌ Columna %d inválida.\n", i + 1);
            valid = 0;
        }
    }
    return NULL;
}

// Función para revisar los subcuadrantes 3x3 usando OpenMP
void* check_subgrids(void* arg) {
    printf("Revisando subcuadrantes...\n");
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            int subgrid[SIZE];
            int index = 0;
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 3; l++) {
                    subgrid[index++] = sudoku[i + k][j + l];
                }
            }
            if (!validate_array(subgrid)) {
                printf("❌ Subgrid en (%d, %d) inválido.\n", i + 1, j + 1);
                valid = 0;
            }
        }
    }
    return NULL;
}
