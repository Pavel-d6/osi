#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    double re, im;
} Complex;

typedef struct {
    const Complex *A, *B;   
    Complex *C;            
    int M, K;              
    int row_begin;         
    int row_end;            
} Task;


void *worker(void *arg) {
    Task *t = (Task *)arg;

    for (int i = t->row_begin; i < t->row_end; i++) {
        for (int j = 0; j < t->K; j++) {
            double sum_re = 0.0, sum_im = 0.0;

            for (int k = 0; k < t->M; k++) {
                Complex a = t->A[i * t->M + k];
                Complex b = t->B[k * t->K + j];

                sum_re += a.re * b.re - a.im * b.im;
                sum_im += a.re * b.im + a.im * b.re;
            }

            t->C[i * t->K + j].re = sum_re;
            t->C[i * t->K + j].im = sum_im;
        }
    }
    return NULL;
}

void fill_random(Complex *X, int count) {
    for (int i = 0; i < count; i++) {
        X[i].re = 2.0 * rand() / RAND_MAX - 1.0;   
        X[i].im = 2.0 * rand() / RAND_MAX - 1.0;
    }
}


double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    int threads = 1;
    int N = 500;

    int opt;
    while ((opt = getopt(argc, argv, "t:n:")) != -1) {
        switch (opt) {
        case 't': threads = atoi(optarg); break;
        case 'n': N = atoi(optarg); break;
        default:
            fprintf(stderr, "Использование: %s -t потоки -n размер\n", argv[0]);
            return 1;
        }
    }
    if (threads < 1 || N < 1) {
        fprintf(stderr, "Некорректные параметры\n");
        return 1;
    }
    int M = N, K = N;   

    Complex *A = malloc((size_t)N * M * sizeof(Complex));
    Complex *B = malloc((size_t)M * K * sizeof(Complex));
    Complex *C = malloc((size_t)N * K * sizeof(Complex));
    if (!A || !B || !C) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    srand(1);
    fill_random(A, N * M);
    fill_random(B, M * K);

    int p = threads;
    if (p > N) p = N;   

    pthread_t *tid = malloc(p * sizeof(pthread_t));
    Task *tasks = malloc(p * sizeof(Task));

    double t0 = now_sec(); 

    int base = N / p, rest = N % p, row = 0;
    for (int i = 0; i < p; i++) {
        int cnt = base + (i < rest ? 1 : 0);
        tasks[i] = (Task){ A, B, C, M, K, row, row + cnt };
        row += cnt;
        if (pthread_create(&tid[i], NULL, worker, &tasks[i]) != 0) {
            fprintf(stderr, "Ошибка pthread_create\n");
            return 1;
        }
    }

    for (int i = 0; i < p; i++)
        pthread_join(tid[i], NULL);

    double elapsed = now_sec() - t0;            
    printf("Время умножения: %.6f с\n", elapsed);

    printf("Запущено потоков: %d\n", p);
    printf("C[0][0] = %f + %fi\n", C[0].re, C[0].im);

    free(A); free(B); free(C);
    free(tid); free(tasks);
    return 0;
}
