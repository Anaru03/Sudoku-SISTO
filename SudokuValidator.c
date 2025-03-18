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

// Matriz global del Sudoku
int sudoku[SIZE][SIZE];
int valid = 1;

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

    read_sudoku_from_file(argv[1]);
    print_sudoku();
    printf("\n🔍 Revisando el Sudoku...\n\n");
    
    pid_t parent_pid = getpid();
    pid_t pid = fork();
    if (pid == 0) {
        execute_ps_command(parent_pid);
        exit(EXIT_SUCCESS);
    }

    pthread_t tid_columns, tid_rows;
    pthread_create(&tid_columns, NULL, check_columns, NULL);
    pthread_create(&tid_rows, NULL, check_rows, NULL);
    
    pthread_join(tid_columns, NULL);
    pthread_join(tid_rows, NULL);
    
    check_subgrids(NULL);
    
    wait(NULL);
    
    printf("---------------------------------\n");
    if (valid) {
        printf("✅ La solución del Sudoku es VÁLIDA.\n");
    } else {
        printf("❌ La solución del Sudoku es INVÁLIDA.\n");
    }
    return 0;
}

void execute_ps_command(pid_t parent_pid) {
    char parent_pid_str[10];
    sprintf(parent_pid_str, "%d", parent_pid);
    execlp("ps", "ps", "-p", parent_pid_str, "-lLf", NULL);
    perror("Error ejecutando ps");
    exit(EXIT_FAILURE);
}

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

void* check_rows(void* arg) {
    printf("➡️ Revisando filas...\n");
    #pragma omp parallel for schedule(dynamic)
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

void* check_columns(void* arg) {
    printf("➡️ Revisando columnas...\n");
    #pragma omp parallel for schedule(dynamic)
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

void* check_subgrids(void* arg) {
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            int subgrid[SIZE] = {0};
            int index = 0;

            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 3; l++) {
                    subgrid[index++] = sudoku[i + k][j + l];
                }
            }

            if (!validate_array(subgrid)) {
                #pragma omp critical
                {
                    printf("\n❌ Subgrid en (%d, %d) inválido.\n", i + 1, j + 1);
                    for (int k = 0; k < 3; k++) {
                        printf("| ");
                        for (int l = 0; l < 3; l++) {
                            printf("%d ", sudoku[i + k][j + l]);
                        }
                        printf("|\n");
                    }
                    printf("---------------------\n");
                }
            }
        }
    }
    return NULL;
}
